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
        explicit http_connction(boost::asio::io_service &io_service_, http_server &http_server_, http_connection_manager &http_connection_manager_);

        ~http_connction();

    public:
        void start();

        void stop();

        void handle_read_headers(boost::system::error_code ec, std::size_t length);

        void handle_read_body(boost::system::error_code ec, std::size_t length);

        boost::asio::ip::tcp::socket& getSocket();

        boost::asio::ip::tcp::endpoint& getEndpoint();

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
