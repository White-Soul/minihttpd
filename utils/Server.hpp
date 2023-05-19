#pragma once
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <thread>
#include <iostream>
#include <map>
#include <vector>
#include <mutex>
#include <type_traits>
#include "Session.hpp"
#include "Interface.hpp"
#include "ThreadPool.hpp"
#include "Router.hpp"
#include "mysqlconn.hpp"
#include "Servlet.hpp"
#include "Timer.hpp"
#include "def.h"
_HTTPD_BEGIN_

// 服务器类
class HttpServer
{
public:
    // 构造函数，启动一个服务
    HttpServer(boost::asio::io_context &io_context, short port, size_t numThread = 16)
        : io_context(io_context), acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
          thread_pool_(ThreadPool::GetInstance(numThread))
    {
        HttpdLog::split_line();
        std::printf("Httpd服务启动");
        std::printf("版本: %s", VERSION);
        start_accept();
    }
    HttpServer(const HttpServer &) = delete;
    HttpServer &operator=(const HttpServer &) = delete;

    ~HttpServer() {}
    // 等待异步请求
    void run()
    {
        HttpdLog::Info("等待异步请求", (char*)__func__);
        HttpdLog::split_line();
        io_context.run();
    }
    // 绑定路由
    void router(Router &router)
    {
        HttpdLog::Info("绑定路由", (char*)__func__);
        servlet_ = std::make_shared<DispatcherServlet>(router, *this);
    }
    void router(std::initializer_list<std::pair<std::string, HttpServlet *>> list)
    {
        Router router;
        router.Register(list);
        servlet_ = std::make_shared<DispatcherServlet>(router, *this);
    }
    // 绑定数据库
    void database(const std::string &host, const std::string &user,
                  const std::string &password, const std::string &database,
                  unsigned int port = 33060, unsigned int pool_size = 128)
    {
        HttpdLog::Info("绑定数据库", (char*)__func__);
        conn_pool = std::make_shared<ConnectionPool>(host, user, password, database, port, pool_size);
    }
    std::shared_ptr<ConnectionPool> getConnPool()
    {
        return conn_pool;
    }

    Timer getTimer(int max_count = 1)
    {
        return Timer(max_count);
    }

private:
    // 监听请求
    void start_accept()
    {
        try
        {
            HttpdLog::Info("监听请求", (char*)__func__);
            auto socket = std::make_shared<tcp::socket>(io_context);
            acceptor_.async_accept(*socket, boost::bind(&HttpServer::accept_handler,
                                                        this, boost::asio::placeholders::error, socket));
        }
        catch (...)
        {
            handle_excepiton(std::current_exception());
        }
    }
    // 处理函数，
    void accept_handler(const boost::system::error_code &ec, std::shared_ptr<tcp::socket> sock)
    {
        if (ec)
        {
            return;
        }
        HttpdLog::Info("创建Session", (char*)__func__);
        thread_pool_.Enqueue([=]()
                             {
                                try{
                                    boost::shared_ptr<HttpSession> session(new HttpSession(sock, servlet_));
                                    session->start();
                                }catch(...){
                                    handle_excepiton(std::current_exception());
                                } });
        start_accept();
    }
    // io_context
    boost::asio::io_context &io_context;
    // acceptor
    tcp::acceptor acceptor_;
    // servlet
    std::shared_ptr<DispatcherServlet> servlet_;
    // 线程池
    ThreadPool &thread_pool_;
    // 数据库连接池
    std::shared_ptr<ConnectionPool> conn_pool;
    // 定时器
    Timer timer_;
};

_HTTPD_END_