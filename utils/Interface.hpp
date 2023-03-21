#pragma once

#include <iostream>
#include <map>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

class HttpServer;
class HttpSession;
// using namespace boost::property_tree;
using boost::asio::ip::tcp;
// 请求体接口
class HttpServletRequest
{
public:
    enum Method
    {
        GET,
        POST,
        OPTIONS
    };
    virtual ~HttpServletRequest() {}
    // 获取请求方法
    virtual Method get_method() const = 0;
    virtual void set_method(const Method &method) = 0;
    // 获取请求参数
    virtual std::vector<std::pair<std::string, std::string>> get_parameters() const = 0;
    virtual std::string get_parameter(std::string) const = 0;
    // 添加请求头
    virtual void add_header(const std::string &name, const std::string &value) = 0;
    // 添加请求参数
    virtual void add_parameter(std::string key, std::string value) = 0;
    // 获取所有请求头
    virtual std::map<std::string, std::string> get_headers() const = 0;
    virtual std::string get_header(std::string) const = 0;
    // 获取请求路径
    virtual std::string get_path() const = 0;
    virtual void set_path(const std::string &str) = 0;
    // 获取HTTP版本
    virtual std::string get_http_version() const = 0;
    // 打印信息
    virtual void show() const = 0;
    // 获取请求体
    virtual std::string get_body() const = 0;
    // 设置请求体
    virtual void set_body(std::istringstream &) = 0;
    virtual void set_body(std::string &) = 0;
    // 设置token
    virtual std::string get_authorization() const = 0;
};
// 响应体接口
class HttpServletResponse
{
public:
    enum StatusCode
    {
        OK = 200,
        BAD_REQUEST = 400,
        NOT_FOUND = 404,
        INTERNAL_SERVER_ERROR = 500,
    };
    virtual ~HttpServletResponse() {}
    // 设置HTTP响应状态码
    virtual void set_status_code(StatusCode status_code) = 0;
    // 设置HTTP响应状态消息；
    virtual void set_status_message(const std::string &status_message) = 0;
    // 设置HTTP响应内容类型
    virtual void set_content_type(const std::string &content_type) = 0;
    // 设置HTTP响应正文；
    virtual void set_body(const std::string &body) = 0;
    virtual void add_body(const std::string &body) = 0;
    // 将HTTP响应对象转换为字符串形式，用于发送给客户端
    virtual std::string to_string() = 0;
    // 设置HTTP响应头；
    virtual void set_header(const std::string &name, const std::string &value) = 0;
    virtual void send() = 0;
    // 设置token
    virtual void set_authorization(std::string&) = 0;
};
// Servlet接口,所有处理类都要继承它
class HttpServlet
{
private:
    HttpServer &server_;
    boost::shared_ptr<HttpSession> session_;

protected:
    boost::shared_ptr<HttpSession> get_session()
    {
        return session_;
    }
    HttpServer& get_server(){
        return server_;
    }

public:
    virtual ~HttpServlet() {}
    HttpServlet(HttpServer &ser) : server_(ser) {}
    // HttpServlet(HttpServlet &ser) = delete;
    virtual void doGet(HttpServletRequest &request, HttpServletResponse &response) = 0;
    virtual void doPost(HttpServletRequest &request, HttpServletResponse &response) = 0;
    void service(HttpServletRequest &request, HttpServletResponse &response, boost::shared_ptr<HttpSession> s)
    {
        session_ = s;
        std::cout << "转发\n";
        response.set_header("Access-Control-Allow-Origin", request.get_header("Origin"));
        response.set_header("Access-Control-Allow-Credentials", "true");
        if (request.get_method() == HttpServletRequest::OPTIONS)
        {
            response.set_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
            response.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
            response.set_status_code(HttpServletResponse::OK);
            response.send();
        }
        else if (request.get_method() == HttpServletRequest::GET)
        {
            this->doGet(request, response);
        }
        else
        {
            this->doPost(request, response);
        }
    }
};

template <class T>
class Custom
{
    virtual std::string serialize() const = 0;
    static T deserialize(const std::string &json){};
};
