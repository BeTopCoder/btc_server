#include "session.h"
#include "session_mgr.h"
#include <iostream>
using namespace std;

namespace http
{
    namespace server
    {
        session::session(boost::asio::io_service & io_service_, sessionMgr &sessionMgr_)
            : m_io_service_(io_service_)
            , m_socket_(io_service_)
            , m_sessionMgr_(sessionMgr_)
            , m_buffer_(new char[8192], [](char *pcTmp) {delete[] pcTmp;})
            , m_RspBuffer_(new char[8192], [](char*pcTmp) {delete[] pcTmp;})
        {
        }

        boost::asio::ip::tcp::socket & session::socket()
        {
            return m_socket_;
        }

        boost::asio::ip::tcp::endpoint & session::endpoint()
        {
            return m_endpoint_;
        }

        void session::start()
        {
            do_read();
        }

        void session::stop()
        {
            m_socket_.close();
        }

        void session::do_read()
        {
            auto self(shared_from_this());
            m_socket_.async_read_some(boost::asio::buffer(m_buffer_.get(), 8192),
                [this, self](boost::system::error_code ec, std::size_t length)
            {
                if (!ec)
                {
                    m_buffer_.get()[length] = '\0';

                    cout << m_buffer_.get() << endl;

                    do_read();
                }
                else if (boost::asio::error::operation_aborted != ec)
                {
                    m_sessionMgr_.stop(self);
                }
                else
                {
                    cout << "something wrong!" << endl;
                    exit(-1);
                }
            });
        }

        void session::do_write()
        {
            auto self(shared_from_this());
            m_socket_.async_write_some(boost::asio::buffer(m_RspBuffer_.get(), 8192),
                [this, self](boost::system::error_code ec, std::size_t ulLength)
            {
                if (!ec)
                {
                    /* ·¢ËÍ³É¹¦ */

                }
                else if (boost::asio::error::operation_aborted != ec)
                {
                    m_sessionMgr_.stop(self);
                }
                else
                {
                    cout << "something wrong!" << endl;
                    exit(-1);
                }
            });
        }

    }
}
