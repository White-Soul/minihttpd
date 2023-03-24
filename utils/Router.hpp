#pragma once
#include <map>
#include <string>
#include <memory>
#include <vector>
#include <initializer_list>
#include "Servlet.hpp"

_HTTPD_BEGIN_

class Router
{
private:
    std::map<std::string, boost::shared_ptr<HttpServlet>> router;

public:
    Router() {}
    Router(std::initializer_list<std::pair<std::string, HttpServlet *>> list)
    {
        for (auto l : list)
        {
            router.insert(std::make_pair(l.first, boost::shared_ptr<HttpServlet>(l.second)));
        }
    }
    Router &operator=(std::initializer_list<std::pair<std::string, HttpServlet *>> list)
    {
        for (auto l : list)
        {
            router.insert(std::make_pair(l.first, boost::shared_ptr<HttpServlet>(l.second)));
        }
    }
    ~Router() {}
    size_t count(std::string path) const
    {
        return router.count(path);
    }
    void destroy()
    {
        router.clear();
    }

    void init()
    {
    }

    void Register(std::string path, HttpServlet *func)
    {
        router[path] = boost::shared_ptr<HttpServlet>(func);
    }
    void Register(std::pair<std::string, HttpServlet *> p)
    {
        router.insert(std::make_pair(p.first, boost::shared_ptr<HttpServlet>(p.second)));
    }
    void Register(std::initializer_list<std::pair<std::string, HttpServlet *>> list)
    {
        for (auto l : list)
        {
            router.insert(std::make_pair(l.first, boost::shared_ptr<HttpServlet>(l.second)));
        }
    }

    boost::shared_ptr<HttpServlet> &getRouter(std::string path)
    {
        return router[path];
    }
    std::vector<std::string> getRouters()
    {
        std::vector<std::string> vec;
        for (auto r = router.begin(); r != router.end(); r++)
        {
            vec.push_back(r->first);
        }
        return vec;
    }
};

_HTTPD_END_
