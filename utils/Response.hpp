#pragma once

#include <boost/asio.hpp>
#include "Interface.hpp"

_HTTPD_BEGIN_

class HttpResponse : public HttpServletResponse
{
private:
    std::vector<std::string> headers_;
    std::string body_;
    StatusCode status_code_;
    std::shared_ptr<boost::asio::ip::tcp::socket> sock_;
    std::string status_message_;

    std::string status_code_string() const
    {
        switch (status_code_)
        {
        case OK:
            return "OK";
        case BAD_REQUEST:
            return "Bad Request";
        case NOT_FOUND:
            return "Not Found";
        case INTERNAL_SERVER_ERROR:
            return "Internal Server Error";
        default:
            return "";
        }
    }

public:
    HttpResponse(std::shared_ptr<boost::asio::ip::tcp::socket> sock) : sock_(sock) {}
    HttpResponse(StatusCode status_code, std::shared_ptr<boost::asio::ip::tcp::socket> sock)
        : status_code_(status_code), sock_(sock)
    {
        status_message_ = status_code_string();
    }
    void set_status_code(StatusCode status_code) override
    {
        status_code_ = status_code;
        status_message_ = status_code_string();
    }

    void set_content_type(const std::string &content_type) override
    {
        add_header("Content-Type: " + content_type);
    }

    void set_status_message(const std::string &status_message) override
    {
        status_message_ = status_message;
    }

    void add_header(const std::string &header)
    {
        headers_.push_back(header);
    }

    void set_body(const std::string &body) override
    {
        body_ = body;
    }

    std::string to_string() override
    {
        std::string response_str;
        response_str = "HTTP/1.1 " + std::to_string(status_code_) + " " + status_code_string() + "\r\n";
        for (const auto &header : headers_)
        {
            response_str += header + "\r\n";
        }
        response_str += "Content-Length: " + std::to_string(body_.size()) + "\r\n\r\n";
        return response_str;
    }
    void send() override
    {
        boost::asio::streambuf response;
        // 响应头
        std::ostream response_stream(&response);
        response_stream << to_string();

        // 发送响应头
        boost::asio::write(*sock_, response);

        // 发送响应体
        if (!body_.empty())
        {
            boost::asio::write(*sock_, boost::asio::buffer(body_));
        }
    }
    void set_header(const std::string &name, const std::string &value) override
    {
        add_header(name + ": " + value);
    }
    void add_body(const std::string &body) override
    {
        body_ += body;
    }
    void set_authorization(std::string &str) override
    {
        add_header("Authorization: " + str);
    }
};

_HTTPD_END_