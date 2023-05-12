#pragma once

#include <iostream>
#include <map>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/algorithm/string.hpp>
#include "Interface.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Servlet.hpp"
#include "utils.hpp"

_HTTPD_BEGIN_

// 会话
class HttpSession : public boost::enable_shared_from_this<HttpSession>
{
private:
    std::string sessionId_;
    std::map<std::string, std::string> attributes_;
    std::shared_ptr<boost::asio::ip::tcp::socket> socket_;
    /*=============================*/
    std::shared_ptr<DispatcherServlet> dispatcher_;
    std::shared_ptr<HttpServletRequest> request;
    std::shared_ptr<HttpServletResponse> response;

    // 报文缓冲区
    std::vector<char> buffer;
    // 报文
    std::string request_str;
    // 标志位避免多次回调
    bool data_flag;

public:
    HttpSession(std::shared_ptr<boost::asio::ip::tcp::socket> sock, std::shared_ptr<DispatcherServlet> servlet) : socket_(sock), dispatcher_(servlet)
    {
        // sessionId_ = 100;
        buffer.resize(1024);
        data_flag = false;
    }
    // 启动一个会话
    void start()
    {
        if (!data_flag)
        {
            data_flag = true;
            auto self(shared_from_this());
            socket_->async_read_some(boost::asio::buffer(buffer),
                                     boost::bind(&HttpSession::read, self,
                                                 boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
        }
    }
    // 读取请求头
    void read(const boost::system::error_code &error,
              size_t bytes_transferred)
    {
        HttpdLog::Info("读取Header");
        auto self(shared_from_this());
        if (error)
            return;
        request_str += std::string(buffer.begin(), buffer.begin() + bytes_transferred);
        int index = request_str.find("\r\n\r\n", 0);
        // 读取结束
        if (-1 != index)
        {
            // 分发请求
            HttpdLog::Info("分发请求");
            this->request = parse_request_header(request_str);
            this->response = std::make_shared<HttpResponse>(this->socket_);
            dispatcher_->service(*request, *response, self);
            data_flag = false;
            return;
        }
        else
        {
            socket_->async_read_some(boost::asio::buffer(buffer),
                                     boost::bind(&HttpSession::read, self,
                                                 boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
        }
    }
    // 获取sessionid
    std::string getId()
    {
        return sessionId_;
    }
    // 获取key-value对
    std::string getAttribute(const std::string &key) const
    {
        if (attributes_.count(key))
        {
            return attributes_.at(key);
        }
        return "";
    }
    // 设置key-value对
    void setAttribute(const std::string &key, const std::string &value)
    {
        attributes_[key] = value;
    }
    // 移除kay-value对
    void removeAttribute(const std::string &key)
    {
        attributes_.erase(key);
    }
    ~HttpSession()
    {
    }
};

_HTTPD_END_
