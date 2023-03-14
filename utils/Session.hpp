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
        std::cout << "启动一个会话\n";
        while (true)
        {
            if (!data_flag)
            {
                data_flag = true;
                auto self(shared_from_this());
                socket_->async_read_some(boost::asio::buffer(buffer),
                                         boost::bind(&HttpSession::read, self,
                                                     boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
            }
            else
            {
                break;
            }
        }
    }
    // 读取请求头
    void read(const boost::system::error_code &error,
              size_t bytes_transferred)
    {
        std::cout << "读取请求头\n";
        if (error)
            return;
        request_str += std::string(buffer.begin(), buffer.begin() + bytes_transferred);
        int index = request_str.find("\r\n\r\n", 0);
        // 读取结束
        if (-1 != index)
        {
            // 分发请求
            std::cout << "分发请求\n";
            data_flag = false;
            this->request = parse_request_header(request_str);
            this->response = std::make_shared<HttpResponse>(this->socket_);
            dispatcher_->service(*request, *response);
            return;
        }

        auto self(shared_from_this());
        socket_->async_read_some(boost::asio::buffer(buffer),
                                 boost::bind(&HttpSession::read, self,
                                             boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }
    // 解析HTTP请求头
    std::shared_ptr<HttpServletRequest> parse_request_header(const std::string &request_str)
    {
        std::cout << "解析HTTP请求头\n";
        // 分离请求行和请求头
        auto headers_start = request_str.find("\r\n");
        if (headers_start == std::string::npos)
        {
            throw std::runtime_error("Invalid request: no headers found");
        }
        std::string request_line = request_str.substr(0, headers_start);
        auto body_start = request_str.find("\r\n\r\n");
        std::string headers = request_str.substr(headers_start + 2, body_start);

        // 解析请求行
        std::cout << "解析请求行\n";
        std::vector<std::string> tokens;
        boost::split(tokens, request_line, boost::is_any_of(" "));
        if (tokens.size() != 3)
        {
            throw std::runtime_error("Invalid request: malformed request line");
        }
        auto method = tokens[0] == "GET" ? HttpServletRequest::GET : (tokens[0] == "POST" ? HttpServletRequest::POST : HttpServletRequest::OPTIONS);
        auto uri = tokens[1];
        auto version = tokens[2];

        std::shared_ptr<HttpServletRequest> request(new HttpRequest(method, uri, version));

        // 解析请求参数
        std::cout << "解析请求参数\n";
        auto query_start = uri.find('?');
        if (query_start != std::string::npos)
        {
            std::string query_string = uri.substr(query_start + 1);
            std::vector<std::string> params;
            boost::split(params, query_string, boost::is_any_of("&"));
            for (auto &param : params)
            {
                std::vector<std::string> kv;
                boost::split(kv, param, boost::is_any_of("="));
                if (kv.size() == 2)
                {
                    (request)->add_parameter(kv[0], kv[1]);
                }
            }
            uri = uri.substr(0, query_start);
        }

        // 解析请求头
        std::vector<std::string> header_lines;
        std::cout << "=========headers============\n";
        std::cout << headers + "\n";
        boost::split(header_lines, headers, boost::is_any_of("\r\n"));
        for (auto &header : header_lines)
        {
            auto delimiter_pos = header.find(":");
            if (delimiter_pos != std::string::npos)
            {
                std::string header_name = header.substr(0, delimiter_pos);
                std::string header_value = header.substr(delimiter_pos + 1);
                boost::algorithm::trim(header_value);
                request->add_header(header_name, header_value);
            }
        }
        if (request->get_method() == HttpServletRequest::OPTIONS)
            return request;
        // 解析请求体（仅对POST请求有效）
        if (method == HttpServletRequest::POST)
        {
            std::cout << "解析请求体==仅对POST请求有效\n";
            auto content_type = request->get_header("Content-Type");
            if (content_type == "application/json")
            {
                if (body_start != std::string::npos)
                {
                    auto body = request_str.substr(body_start + 4);
                    std::istringstream is(body);
                    request->set_body(is);
                }
            }
            else if (content_type == "application/x-www-form-urlencoded")
            {
                auto body_start = request_str.find("\r\n\r\n");
                if (body_start != std::string::npos)
                {
                    auto body = request_str.substr(body_start + 4);
                    std::vector<std::string> params;
                    boost::split(params, body, boost::is_any_of("&"));
                    for (auto &param : params)
                    {
                        std::vector<std::string> kv;
                        boost::split(kv, param, boost::is_any_of("="));
                        if (kv.size() == 2)
                        {
                            request->add_parameter(kv[0], kv[1]);
                        }
                    }
                }
            }
            else
            {
                throw std::runtime_error("Unsupported Content-Type: " + content_type);
            }
        }
        std::cout << "解析完毕\n";
        return request;
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
};
