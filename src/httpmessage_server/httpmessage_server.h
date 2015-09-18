#ifndef __HTTPMESSAGE_SERVER_H__
#define __HTTPMESSAGE_SERVER_H__

#include <boost/asio.hpp>

namespace httpmessage_server
{
    class http_server
        : public boost::noncopyable
    {
    public:
        explicit http_server(boost::asio::io_service &io_servce_, unsigned short usPort, std::string address = "0.0.0.0");

        ~http_server();

    public:
        void init();

        void start();

        void stop();

    protected:
    private:
        boost::asio::io_service &m_io_service_;
        boost::asio::ip::tcp::acceptor m_acceptor_;
        http_connection_ptr m_http_connection_ptr_;
        http_connection_manager m_http_connection_manager_;

        unsigned short m_usPort;
        std::string m_strAddress_;
    };
}

#endif
