#include "server.h"
#include "session.h"
#include <iostream>
using namespace std;

namespace http
{
    namespace server
    {
        server::server(std::string &strIPAddress, std::string &strPort)
            : m_io_service_()
            , m_acceptor_(m_io_service_)
        {
            using namespace boost::asio::ip;
            tcp::resolver resolver_(m_io_service_);
            tcp::endpoint endpoint_ = *resolver_.resolve({ strIPAddress, strPort });
            m_acceptor_.open(endpoint_.protocol());
            m_acceptor_.set_option(tcp::acceptor::reuse_address(true));
            m_acceptor_.bind(endpoint_);
            m_acceptor_.listen();

            do_acceptor();
        }

        void server::run()
        {
            m_io_service_.run();
        }

        void server::do_acceptor()
        {
            std::shared_ptr<session> session_(new session(m_io_service_, m_sessionMgr));

            m_acceptor_.async_accept(session_->socket(), session_->endpoint(),
                [this, session_](boost::system::error_code ec) 
            {
                if (!m_acceptor_.is_open())
                {
                    return;
                }

                if (!ec)
                {
                    m_sessionMgr.start(session_);
                }
                else
                {
                    return;
                }

                do_acceptor();
            });
        }


    }
}
