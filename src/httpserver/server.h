#ifndef __SERVER_H__
#define __SERVER_H__

#include <string>
#include <boost/asio.hpp>
#include "session_mgr.h"

namespace http
{
    namespace server
    {
        class server
        {
        public:
            server(const server&) = delete;
            server& operator=(const server&) = delete;

            server(std::string &strIPAddress, std::string &strPort);

            void run();
        protected:

        private:
            void do_acceptor();

        private:
            boost::asio::io_service m_io_service_;

            boost::asio::ip::tcp::acceptor m_acceptor_;

            sessionMgr m_sessionMgr;
        };
    }
}

#endif
