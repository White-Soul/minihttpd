#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include "utils.hpp"
#define THREADNUM 128

_HTTPD_BEGIN_
// 线程池
class ThreadPool
{
public:
    // 获取线程池，单例模式
    static ThreadPool &GetInstance(unsigned int num = THREADNUM)
    {
        static ThreadPool instance;
        instance.Init(num);
        return instance;
    }

    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }
        condition_.notify_all();
        for (std::thread &worker : threads_)
            worker.join();
    }
    // 加入任务
    template <class F>
    void Enqueue(F &&f)
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            tasks_.emplace(std::forward<F>(f));
        }
        condition_.notify_one();
    }
    // 调整线程池
    void resize(size_t numThreads);

private:
    void Init(size_t numThreads);

    ThreadPool() = default;
    ThreadPool(const ThreadPool &) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;
    // 队列
    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> tasks_;
    // 线程同步
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_ = false;
};

void ThreadPool::Init(size_t numThreads)
{
    for (size_t i = 0; i < numThreads; ++i)
    {
        threads_.emplace_back([this]
                              {
            try{           
                for (;;) {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex_);
                        this->condition_.wait(lock, [this] { return this->stop_ || !this->tasks_.empty(); });
                        if (this->stop_ && this->tasks_.empty())
                            return;
                        task = std::move(this->tasks_.front());
                        this->tasks_.pop();
                    }
                        task();
                } 
            }catch(...){
                handle_excepiton(std::current_exception());
            }});
    }
}

void ThreadPool::resize(size_t numThreads)
{
    if (numThreads == threads_.size())
    {
        return;
    }

    if (numThreads > threads_.size())
    {
        for (size_t i = threads_.size(); i < numThreads; ++i)
        {
            threads_.emplace_back([this]
                                  {
                for (;;) {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex_);
                        this->condition_.wait(lock, [this] { return this->stop_ || !this->tasks_.empty(); });
                        if (this->stop_ && this->tasks_.empty())
                            return;
                        task = std::move(this->tasks_.front());
                        this->tasks_.pop();
                    }

                    task();
                } });
        }
    }
    else
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }
        condition_.notify_all();

        for (auto &thread : threads_)
        {
            thread.join();
        }

        threads_.resize(numThreads);
        stop_ = false;

        for (size_t i = 0; i < threads_.size(); ++i)
        {
            condition_.notify_one();
        }
    }
}

_HTTPD_END_