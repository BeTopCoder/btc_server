#include <iostream>
#include "httpmessage_server.h"
#include "logging.hpp"
using namespace std;

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
    }

    void http_server::stop()
    {
    }
}


int main(int argc, char **argv)
{
    INIT_LOGGER("example.log");
    LOG_ERR << "123";
    return 0;
}