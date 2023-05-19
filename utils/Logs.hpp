#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <mysqlx/xdevapi.h>
#include <boost/format.hpp>
#include "def.h"

namespace httpd
{
    class HttpdLog
    {
    public:
        static void Info(const std::string &msg, const std::string &func)
        {
            auto m = boost::format("[INFO] [%2%] %1% \n") % msg % func;
            std::printf(m.str().c_str());
        }
        static void Warn(const std::string& msg, const std::string& func)
        {
            auto m = boost::format("[WARN] [%2%] %1% \n") % msg % func;
            std::printf(m.str().c_str());
        }
        static void Error(const std::string& msg, const std::string &func)
        {
            auto m = boost::format("[WARN] [%2%] %1% \n") % msg % func;
            std::printf(m.str().c_str());
        }
        static void split_line()
        {
            std::cout << "===============================" << std::endl;
        }
        static std::string format(const char* str, std::string args...){
            auto fmt = const_cast<char*>(str);
        }

        static HttpdLog& getSingleLog(){
            return SingleLog;
        }

        void write(const std::string &msg, const std::string &func)
        {
            auto m = boost::format("[WARN] [%2%] %1% \n") % msg % func;
            std::ofstream out(_filename, std::ofstream::app);
            auto str = m.str();
            if (out.is_open())
                out << str;
        }

    private:
        static HttpdLog SingleLog;
        HttpdLog(const std::string &s) : _filename(s) {}
        HttpdLog(const char *s) : _filename(s) {}
        std::string _filename;
    };
    HttpdLog HttpdLog::SingleLog{FILENAME};
} // namespace httpd
