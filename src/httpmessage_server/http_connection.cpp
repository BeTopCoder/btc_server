#include "http_connection.hpp"
#include "logging.hpp"
#include <boost/bind.hpp>

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
        m_request_.consume(m_request_.size());

        boost::system::error_code ec;
        m_socket_.set_option(boost::asio::ip::tcp::no_delay(true), ec);
        if (ec)
        {
            LOG_ERR << "http_connection:start, Set Options to nodelay, error message: " << ec.message();
            throw std::exception("error:set Options");
        }
        LOG_ERR << "http_connection start";
        boost::asio::async_read_until(m_socket_, m_request_, "\r\n\r\n", boost::bind(&http_connction::handle_read_headers, shared_from_this(), _1, _2));
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
        m_request_.sgetn(&buffer_[0], length);
        LOG_ERR << *(buffer_.begin());
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
        return;
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