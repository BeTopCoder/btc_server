#ifndef __HTTP_CONNECTION_H__
#define __HTTP_CONNECTION_H__

#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <memory>
#include <mutex>
#include <set>

namespace httpmessage_server
{
    class http_server;
    class http_connection_manager;
    class http_connction
        : public std::enable_shared_from_this<http_connction>
        , public boost::noncopyable
    {
    public:
        explicit http_connction(boost::asio::io_service &io_service_, http_server &http_server_, http_connection_manager &http_connection_manager_)
            : m_io_service_(io_service_)
            , m_socket_(m_io_service_)
            , m_http_server_(http_server_)
            , m_abort(false)
            , m_http_connection_manager_(http_connection_manager_)
        {

        }

    public:
        void start()
        {
            m_request_.consume(m_request_.size());

            boost::system::error_code ec;
            m_socket_.set_option(boost::asio::ip::tcp::no_delay(true), ec);
            if (ec)
            {
                LOG_ERR << "http_connection:start, Set Options to nodelay, error message: " << ec.message();
                throw std::exception("error:set Options");
            }
            auto self = shared_from_this();
            boost::asio::async_read(m_socket_, m_request_, [this, self](boost::system::error_code ec, std::size_t length) {
                if (ec || m_abort)
                {
                    m_http_connection_manager_.stop(shared_from_this());
                    return;
                }

                /* ¸´ÖÆhttpÍ·»º³åÇø */
                std::vector<char> buffer_;
                buffer_.resize(length + 1);
                buffer_[length] = 0;
                m_request_.sgetn(&buffer_[0], length);
            });
        }

        void stop()
        {
            boost::system::error_code ec;
            m_abort = true;
            m_socket_.close(ec);
        }

        boost::asio::ip::tcp::socket& getSocket()
        {
            return m_socket_;
        }

        boost::asio::ip::tcp::endpoint& getEndpoint()
        {
            return m_endpoint_;
        }

    protected:
    private:
        boost::asio::io_service &m_io_service_;
        boost::asio::ip::tcp::socket m_socket_;
        boost::asio::ip::tcp::endpoint m_endpoint_;
        boost::asio::streambuf m_request_;
        http_server &m_http_server_;
        bool m_abort;

        http_connection_manager &m_http_connection_manager_;
    };

    typedef std::shared_ptr<http_connction> http_connection_ptr;

    class http_connection_manager
        : public boost::noncopyable
    {
    public:
        explicit http_connection_manager()
        {

        }

        void start(http_connection_ptr http_connection_)
        {
            m_mutex_.lock();
            m_http_connection_ptrs_.insert(http_connection_);
            http_connection_->start();
            m_mutex_.unlock();
        }

        void stop(http_connection_ptr http_connection_)
        {
            m_mutex_.lock();
            http_connection_->stop();
            m_http_connection_ptrs_.erase(http_connection_);
            m_mutex_.unlock();
        }

        void stopAll()
        {
            m_mutex_.lock();
            for (auto http_connection_ : m_http_connection_ptrs_)
            {
                http_connection_->stop();
            }
            m_http_connection_ptrs_.clear();
            m_mutex_.unlock();
        }
    protected:
    private:
        std::mutex m_mutex_;
        std::set<http_connection_ptr> m_http_connection_ptrs_;
    };
}

#endif
