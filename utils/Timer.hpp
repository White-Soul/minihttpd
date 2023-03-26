#pragma once
#include <chrono>
#include <functional>
#include <thread>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <mysqlx/xdevapi.h>
#include "utils.hpp"

namespace httpd
{
    class Timer
    {
    public:
        Timer() : expired_(true), count_(0), max_count_(1) {}
        Timer(int max_count) : expired_(true), count_(0), max_count_(max_count) {}
        Timer(const Timer &t)
        {
            expired_ = t.expired_.load();
            interval_ = t.interval_;
            task_ = t.task_;
            max_count_ = t.max_count_;
            count_ = t.count_.load();
        }
        ~Timer()
        {
            stop();
        }
        void start()
        {
            if (!expired_.exchange(false))
            {
                return;
            }
            count_ = 0;
            thread_ = std::thread(&Timer::run, this);
        }

        void stop()
        {
            expired_.store(true);
            if (thread_.joinable())
            {
                thread_.join();
            }
        }

        void SetInterval(int seconds, std::function<void()> task)
        {
            interval_ = seconds;
            task_ = task;
        }

    private:
        void run()
        {
            while (!expired_)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(interval_));
                task_();
                ++count_;
                if(max_count_ > 0 && count_ >= max_count_){
                    expired_.store(true);
                }
            }
        }
        std::atomic<bool> expired_;
        std::function<void()> task_;
        std::thread thread_;
        int interval_;
        std::atomic<int> count_;
        int max_count_;
    };
}
