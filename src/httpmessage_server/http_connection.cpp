#include "http_connection.hpp"
#include "logging.hpp"

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
        auto self = shared_from_this();
        boost::asio::async_read(m_socket_, m_request_, [this, self](boost::system::error_code ec, std::size_t length) {
            if (ec || m_abort)
            {
                LOG_ERR << ec.message();
                m_http_connection_manager_.stop(shared_from_this());
                return;
            }

            /* ¸´ÖÆhttpÍ·»º³åÇø */
            std::vector<char> buffer_;
            buffer_.resize(length + 1);
            buffer_[length] = 0;
            m_request_.sgetn(&buffer_[0], length);
            LOG_ERR << *(buffer_.begin());
        });
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