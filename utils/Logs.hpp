#pragma once

#include <iostream>
#include <fstream>
#include <string>

namespace httpd
{
    class HttpdLog
    {
    public:
        static void Info(const std::string &msg)
        {
            std::cout << "INFO: " << msg << " ." << std::endl;
        }
        static void Info(const char *msg)
        {
            std::cout << "INFO: " << msg << " ." << std::endl;
        }
        static void Warn(const std::string &msg)
        {
            std::cout << "WARN: " << msg << " !!!" << std::endl;
        }
        static void Warn(const char *msg)
        {
            std::cout << "WARN: " << msg << " !!!" << std::endl;
        }
        static void Error(const std::string &msg)
        {
            std::cerr << "ERROR: >\n"
                      << msg << std::endl;
        }
        static void Error(const char *msg)
        {
            std::cerr << "ERROR: >\n"
                      << msg << std::endl;
        }

        HttpdLog(const std::string &s) : _filename(s) {}
        HttpdLog(const char *s) : _filename(s) {}

        void write(const std::string &msg)
        {
            std::ofstream out(_filename, std::ofstream::app);
            if (out.is_open())
                out << msg;
        }

    private:
        std::string _filename;
    };
} // namespace httpd
