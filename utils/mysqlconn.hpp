#pragma once
#include <mysqlx/xdevapi.h>
#include <queue>
#include <mutex>
#include <condition_variable>

class ConnectionPool
{
public:
    ConnectionPool(const std::string &host, const std::string &user,
                   const std::string &password, const std::string &database,
                   unsigned int port = 33060, unsigned int pool_size = 32)
        : host_(host), user_(user), password_(password), database_(database),
          port_(port), pool_size_(pool_size), count_(0)
    {
        if(pool_size_ > 32) pool_size_ = 32;
        for (unsigned int i = 0; i < pool_size_; ++i)
        {
            connections_.emplace(createConnection());
        }
        std::cout << "数据库初始化" << std::endl;
    }

    ConnectionPool()
    {
    }

    ~ConnectionPool()
    {
        while (!connections_.empty())
        {
            connections_.front()->close();
            connections_.pop();
        }
    }

    ConnectionPool(const ConnectionPool &) = delete;
    ConnectionPool &operator=(const ConnectionPool &) = delete;

    std::shared_ptr<mysqlx::Session> getConnection()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (connections_.empty())
        {
            cv_.wait(lock);
        }
        auto conn = connections_.front();
        connections_.pop();
        ++count_;
        return conn;
    }

    void releaseConnection(std::shared_ptr<mysqlx::Session> conn)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        connections_.emplace(std::move(conn));
        --count_;
        cv_.notify_one();
    }

    unsigned int getPoolSize() const
    {
        return pool_size_;
    }

    void setPoolSize(unsigned int pool_size)
    {
        pool_size_ = pool_size;
    }

    std::string getHost() const
    {
        return host_;
    }

    void setHost(const std::string &host)
    {
        host_ = host;
    }

    std::string getUser() const
    {
        return user_;
    }

    void setUser(const std::string &user)
    {
        user_ = user;
    }

    std::string getPassword() const
    {
        return password_;
    }

    void setPassword(const std::string &password)
    {
        password_ = password;
    }

    std::string getDatabase() const
    {
        return database_;
    }

    void setDatabase(const std::string &database)
    {
        database_ = database;
    }

    unsigned int getPort() const
    {
        return port_;
    }

    void setPort(unsigned int port)
    {
        port_ = port;
    }

private:
    std::shared_ptr<mysqlx::Session> createConnection()
    {
        auto conn = std::make_shared<mysqlx::Session>(host_, port_, user_, password_, database_);
        return conn;
    }

    std::queue<std::shared_ptr<mysqlx::Session>> connections_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::string host_;
    std::string user_;
    std::string password_;
    std::string database_;
    unsigned int port_;
    unsigned int pool_size_;
    unsigned int count_;
};
