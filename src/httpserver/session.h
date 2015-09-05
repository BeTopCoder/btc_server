#ifndef __SESSION_H__
#define __SESSION_H__

#include <memory>
#include <boost/asio.hpp>
#include <array>

namespace http
{
    namespace server
    {
        class sessionMgr;

        class session : public std::enable_shared_from_this<session>
        {
        public:
            session(const session&) = delete;
            session& operator=(const session &) = delete;

            explicit session(boost::asio::io_service &io_service_, sessionMgr &sessionMgr_);

            boost::asio::ip::tcp::socket& socket();

            boost::asio::ip::tcp::endpoint& endpoint();

            void start();

            void stop();
        protected:

        private:
            void do_read();

            void do_write();

        private:
            boost::asio::io_service &m_io_service_;

            boost::asio::ip::tcp::socket m_socket_;

            boost::asio::ip::tcp::endpoint m_endpoint_;

            std::shared_ptr<char> m_buffer_;

            sessionMgr &m_sessionMgr_;
        };

        typedef std::shared_ptr<session> session_ptr;
    }
}

#endif

