#ifndef __HTTPMESSAGE_SERVER_H__
#define __HTTPMESSAGE_SERVER_H__


namespace httpmessage_server
{
    typedef boost::function<void(const request&, http_connection_ptr, http_connection_manager&)> http_request_callback;
    typedef std::map<std::string, http_request_callback> http_request_callback_table;
    class http_server
        : public boost::noncopyable
    {
        friend class http_connection;
    public:
        explicit http_server(boost::asio::io_service &io_servce_, unsigned short usPort, std::string address = "0.0.0.0");

        ~http_server();

    public:
        void init();

        void start();

        void stop();

        bool handle_request(request &request_, http_connection_ptr conn);

        bool add_handle(const std::string &uri, http_request_callback cb);

    protected:
    private:
        boost::asio::io_service &m_io_service_;
        boost::asio::ip::tcp::acceptor m_acceptor_;
        http_connection_ptr m_http_connection_ptr_;
        http_connection_manager m_http_connection_manager_;

        /* 处理Get请求的回掉函数表和锁 */
        boost::shared_mutex m_callback_funcs_mutex_;
        http_request_callback_table m_callback_funcs_;

        /* 监听的服务器端口和地址 */
        unsigned short m_usPort;
        std::string m_strAddress_;
    };
}

#endif
