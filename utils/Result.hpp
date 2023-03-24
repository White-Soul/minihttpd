#pragma once
#include <string>
#include "Interface.hpp"

_HTTPD_BEGIN_
enum Result_Code
{
    OK_,
    INFO_,
    WARN_,
    ERROR_
};

class Result
{
private:
    std::string message_;
    std::string data_;
    Result_Code code_;

public:
    Result() {}
    Result(Result_Code code) : code_(code) {}
    Result(std::string &str) : message_(str) {}
    Result(const char *str) : message_(str) {}
    Result(Result_Code code, std::string &str) : code_(code), message_(str) {}
    Result(Result_Code code, const char *str) : code_(code), message_(str) {}
    ~Result() {}
    // get 方法
    std::string get_Message() const { return message_; }
    std::string get_Data() const { return data_; }
    Result_Code get_Result_Code() const { return code_; }
    std::string get_Result_Code_str();
    // set
    void set_Message(std::string &str) { message_ = str; }
    void set_Message(const char *str) { message_ = str; }
    void set_Data(std::string &data) { data_ = data; }
    void set_Data(std::string &&data) { data_ = data; }
    void set_Data(const char *data) { data_ = data; }
    void set_Result_Code(Result_Code code) { code_ = code; }
};

std::string Result::get_Result_Code_str()
{
    switch (this->code_)
    {
    case Result_Code::OK_:
        return "OK";
        break;
    case Result_Code::INFO_:
        return "INFO";
        break;
    case Result_Code::WARN_:
        return "WARN";
        break;
    case Result_Code::ERROR_:
        return "ERROR";
        break;

    default:
        return "";
        break;
    }
}

_HTTPD_END_
