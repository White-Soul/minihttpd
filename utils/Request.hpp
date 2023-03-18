#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "Interface.hpp"

class HttpRequest : public HttpServletRequest
{
public:
    // 构造函数
    HttpRequest(Method method, std::string path, std::string http_version)
        : method_(method), path_(path), http_version_(http_version) {}

    // 获取请求方法
    Method get_method() const override
    {
        return method_;
    }
    void set_method(const Method& method) override{
        method_ = method;
    }

    // 获取请求路径
    std::string get_path() const override
    {
        return path_;
    }
    void set_path(const std::string& str) override{
        path_ = str;
    }

    // 获取HTTP版本
    std::string get_http_version() const override
    {
        return http_version_;
    }

    // 添加请求头
    void add_header(const std::string &name, const std::string &value) override
    {
        headers_[name] = value;
    }

    // 获取所有请求头
    std::map<std::string, std::string> get_headers() const override
    {
        return headers_;
    }
    std::string get_header(std::string name) const override
    {
        return headers_.at(name);
    }

    // 添加请求参数
    void add_parameter(std::string key, std::string value) override
    {
        parameters_.push_back({key, value});
    }

    // 获取请求参数
    std::vector<std::pair<std::string, std::string>> get_parameters() const override
    {
        return parameters_;
    }

    std::string get_parameter(std::string name) const override
    {
        for (auto p : parameters_)
        {
            if (p.first == name)
                return p.second;
        }
        return "";
    }

    friend std::ostream &operator<<(std::ostream &os, const HttpRequest &req);

    std::string get_body() const override
    {
        return body;
    }

    void set_body(std::istringstream &is) override
    {
        boost::property_tree::ptree pt;
        read_json(is, pt);
        body = is.str();
    }
    void set_body(std::string &body) override
    {
        body = body;
    }

    void show() const override
    {
        std::cout << *this;
    }

private:
    Method method_;                                               // 请求方法
    std::string path_;                                            // 请求路径
    std::string http_version_;                                    // HTTP版本
    std::map<std::string, std::string> headers_;                  // 请求头
    std::vector<std::pair<std::string, std::string>> parameters_; // 请求参数
    std::string body;
};

std::ostream &operator<<(std::ostream &os, const HttpRequest &req)
{
    os << "Method: " << (req.method_ == HttpRequest::GET ? "GET" : "POST") << std::endl;
    os << "Path: " << req.path_ << std::endl;
    os << "HTTP Version: " << req.http_version_ << std::endl;
    os << "Headers:\n";
    for (auto h = req.headers_.begin(); h != req.headers_.end(); ++h)
    {
        os << h->first << " : " << h->second << std::endl;
    }
    os << "Parameters:\n";
    for (auto p : req.parameters_)
    {
        os << p.first << " : " << p.second << std::endl;
    }
    return os;
}