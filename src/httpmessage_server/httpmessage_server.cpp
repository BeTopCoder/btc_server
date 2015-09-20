#include <boost/bind.hpp>
#include <boost/integer_traits.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>

#include <iostream>
#include <mutex>
#include <set>
using namespace std;

#include "http_helper.hpp"
#include "logging.hpp"
#include "http_connection.hpp"
#include "httpmessage_server.h"


namespace httpmessage_server
{
    http_server::http_server(boost::asio::io_service &io_servce_, unsigned short usPort, std::string address)
        : m_io_service_(io_servce_)
        , m_acceptor_(m_io_service_)
        , m_usPort(usPort)
        , m_strAddress_(address)
    {

    }
    http_server::~http_server()
    {
    }

    void http_server::init()
    {
        boost::asio::ip::tcp::resolver resolver_(m_io_service_);
        stringstream strPort_;
        strPort_ << m_usPort;
        boost::system::error_code ec;
        boost::asio::ip::tcp::resolver::query query_(m_strAddress_, strPort_.str());
        boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver_.resolve(query_, ec);
        if (ec)
        {
            LOG_ERR << "HTTP Server bind address, DNS resolve failed" << ec.message() << ", address:" << m_strAddress_;
            throw std::exception("DNS resolve failed");
        }

        boost::asio::ip::tcp::endpoint endpoint_ = *endpoint_iterator;
        m_acceptor_.open(endpoint_.protocol(), ec);
        if (ec)
        {
            LOG_ERR << "HTTP Server open protocol failed: " << ec.message();
            throw std::exception("HTTP Server open protocol failed");
        }

        m_acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), ec);
        if (ec)
        {
            LOG_ERR << "HTTP Server open protocol failed: " << ec.message();
            throw std::exception("HTTP Server open protocol failed");
        }

        m_acceptor_.bind(endpoint_, ec);
        if (ec)
        {
            LOG_ERR << "HTTP Server bind failed: " << ec.message() << ", address: " << m_strAddress_;
            throw std::exception("HTTP bind failed");
        }

        m_acceptor_.listen(boost::asio::socket_base::max_connections, ec);
        if (ec)
        {
            LOG_ERR << "HTTP Server listen failed: " << ec.message();
            throw std::exception("HTTP Server listen failed");
        }
    }

    void http_server::start()
    {
        m_http_connection_ptr_ = std::make_shared<http_connction>(std::ref(m_io_service_), std::ref(*this), std::ref(m_http_connection_manager_));
        m_acceptor_.async_accept(m_http_connection_ptr_->getSocket(), m_http_connection_ptr_->getEndpoint(), [this](boost::system::error_code ec)
        {
            if (!m_acceptor_.is_open() || ec)
            {
                if (ec)
                {
                    LOG_ERR << "http_server::async_accept,error" << ec.message();
                    return;
                }
            }

            m_http_connection_manager_.start(m_http_connection_ptr_);

            start();
        });
    }

    void http_server::stop()
    {
        m_acceptor_.close();
        m_http_connection_manager_.stopAll();
        return;
    }

    bool http_server::handle_request(request &request_, http_connection_ptr conn)
    {
        LOG_ERR << request_.uri;
        /* 根据URI调用不同的处理 */
        const std::string &uri = request_.uri;
        boost::shared_lock<boost::shared_mutex> l(m_callback_funcs_mutex_);
        auto iter = m_callback_funcs_.find(uri);
        if (iter == m_callback_funcs_.end())
        {
            return false;
        }
        iter->second(request_, conn, boost::ref(m_http_connection_manager_));
        return true;
    }

    bool http_server::add_handle(const std::string &uri, http_request_callback cb)
    {
        boost::shared_lock<boost::shared_mutex> l(m_callback_funcs_mutex_);
        if (m_callback_funcs_.find(uri) != m_callback_funcs_.end())
        {
            return false;
        }
        m_callback_funcs_[uri] = cb;
        return true;
    }
}


int main(int argc, char **argv)
{
    INIT_LOGGER("example.log");
    LOG_ERR << "123";
    boost::asio::io_service io_service_;

    using namespace httpmessage_server;
    http_server http_server_(io_service_, 10002);
    http_server_.init();
    http_server_.start();
    io_service_.run();

    return 0;
}