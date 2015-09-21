#include <boost/bind.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include <string>
#include <memory>
#include <mutex>
#include <set>

#include "logging.hpp"
#include "http_helper.hpp"

#include "http_connection.hpp"
#include "httpmessage_server.h"

namespace httpmessage_server
{
    http_connction::http_connction(boost::asio::io_service &io_service_, http_server &http_server_, http_connection_manager &http_connection_manager_)
        : m_io_service_(io_service_)
        , m_socket_(m_io_service_)
        , m_http_server_(http_server_)
        , m_abort(false)
        , m_http_connection_manager_(http_connection_manager_)
    {

    }

    http_connction::~http_connction()
    {
        LOG_ERR << "~http_connection";
    }

    void http_connction::start()
    {
        m_recvBuffer_.consume(m_recvBuffer_.size());

        boost::system::error_code ec;
        m_socket_.set_option(boost::asio::ip::tcp::no_delay(true), ec);
        if (ec)
        {
            LOG_ERR << "http_connection:start, Set Options to nodelay, error message: " << ec.message();
            throw std::exception("error:set Options");
        }
        LOG_ERR << "http_connection start";
        boost::asio::async_read_until(m_socket_, m_recvBuffer_, "\r\n\r\n", boost::bind(&http_connction::handle_read_headers, shared_from_this(), _1, _2));
    }

    void http_connction::handle_read_headers(boost::system::error_code ec, std::size_t length)
    {
        if (ec || m_abort)
        {
            LOG_ERR << ec.message();
            m_http_connection_manager_.stop(shared_from_this());
            return;
        }

        LOG_ERR << "==============";

        /* 复制http头缓冲区 */
        std::vector<char> buffer_;
        buffer_.resize(length + 1);
        buffer_[length] = 0;
        m_recvBuffer_.sgetn(&buffer_[0], length);

        boost::tribool result;
        boost::tie(result, boost::tuples::ignore) = m_request_parser.parse(m_request_, buffer_.begin(), buffer_.end());
        if (!result || boost::indeterminate == result)
        {
            /* 断开 */
            m_http_connection_manager_.stop(shared_from_this());
            return;
        }

        m_request_.normalise();

        if (m_request_.method == "post")
        {
            /* 限制Body大小到64KB */
            auto content_length = m_request_.content_length;
            if (content_length == 0 || content_length >= 66536)
            {
                m_http_connection_manager_.stop(shared_from_this());
                return;
            }

            auto already_got = m_recvBuffer_.size();
            if (already_got >= content_length)
            {
                handle_read_body(ec, (size_t)content_length);
            }
            else
            {
                // 读取 body
                boost::asio::async_read(m_socket_, m_recvBuffer_, boost::asio::transfer_exactly(content_length - already_got),
                    boost::bind(&http_connction::handle_read_body, shared_from_this(), _1, _2)
                    );
            }
        }
        else
        {
            if (!m_http_server_.handle_request(m_request_, shared_from_this()))
            {
                /* 断开，反正暴力就对了，越暴力越不容易被攻击 */
                m_http_connection_manager_.stop(shared_from_this());
                return;
            }

            if (m_request_.keep_alive)
            {
                /* 继续读取下一个请求 */
                boost::asio::async_read_until(m_socket_, m_recvBuffer_, "\r\n\r\n", boost::bind(&http_connction::handle_read_headers, shared_from_this(), _1, _2));
            }
        }

        return;
    }

    void http_connction::handle_read_body(boost::system::error_code ec, std::size_t length)
    {
        /* 差错处理 */
        if (ec || m_abort)
        {
            LOG_ERR << ec.message();
            m_http_connection_manager_.stop(shared_from_this());
            return;
        }

        m_request_.body.resize(m_request_.content_length);
        m_recvBuffer_.sgetn(&m_request_.body[0], m_request_.content_length);

        if (!m_http_server_.handle_request(m_request_, shared_from_this()))
        {
            m_http_connection_manager_.stop(shared_from_this());
            return;
        }

        if (m_request_.keep_alive)
        {
            /* 继续下一个请求 */
            boost::asio::async_read_until(m_socket_, m_recvBuffer_, "\r\n\r\n", boost::bind(&http_connction::handle_read_headers, shared_from_this(), _1, _2));
        }

        return;
    }

    void http_connction::write_response(const std::string &body)
    {
        std::ostream out(&m_sendBuffer_);

        out << "HTTP/" << m_request_.http_version_major << "." << m_request_.http_version_minor << " 200 OK\r\n";
        out << "Content-Type: application/json\r\n";
        out << "Content-Length: " << body.length() << "\r\n";
        out << "\r\n";

        out << body;

        boost::asio::async_write(m_socket_, m_sendBuffer_,
            boost::bind(&http_connction::handle_write, shared_from_this(), _1, _2)
            );
    }

    void http_connction::handle_write(boost::system::error_code ec, std::size_t length)
    {
        if (ec || m_abort || !m_request_.keep_alive)
        {
            m_http_connection_manager_.stop(shared_from_this());
            return;
        }
    }

    void http_connction::stop()
    {
        boost::system::error_code ec;
        LOG_ERR << "1234455";
        m_abort = true;
        m_socket_.close(ec);
    }

    boost::asio::ip::tcp::socket& http_connction::getSocket()
    {
        return m_socket_;
    }

    boost::asio::ip::tcp::endpoint& http_connction::getEndpoint()
    {
        return m_endpoint_;
    }
}