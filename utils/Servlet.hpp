#pragma once

#include "Interface.hpp"
#include "Router.hpp"
#include "mysqlconn.hpp"
#include "utils.hpp"
#include <map>

_HTTPD_BEGIN_

class DispatcherServlet : public HttpServlet
{
public:
    void destroy();
    virtual void doGet(HttpServletRequest &request, HttpServletResponse &response) override;
    virtual void doPost(HttpServletRequest &request, HttpServletResponse &response) override;
    virtual ~DispatcherServlet()
    {
        destroy();
    }
    DispatcherServlet(Router &r, HttpServer &server) : router_(r), HttpServlet(server)
    {
    }
    DispatcherServlet(HttpServer &server) : HttpServlet(server) {}

private:
    Router router_;
};

void DispatcherServlet::destroy()
{
    router_.destroy();
}

void DispatcherServlet::doGet(HttpServletRequest &request, HttpServletResponse &response)
{
    HttpdLog::Info("Forward GET");
    std::string path = request.get_path();
    if (router_.count(path) == 0)
    {
        response.set_body("Not Found");
        response.set_status_code(HttpServletResponse::NOT_FOUND);
        response.send();
        return;
    }
    auto servlet = (router_.getRouter(path));
    servlet->doGet(request, response);
}

void DispatcherServlet::doPost(HttpServletRequest &request, HttpServletResponse &response)
{
    HttpdLog::Info("Forward POST");
    std::string path = request.get_path();
    if (router_.count(path) == 0)
    {
        response.set_body("Not Found");
        response.set_status_code(HttpServletResponse::NOT_FOUND);
        response.send();
        return;
    }

    auto servlet = (router_.getRouter(path));
    servlet->doPost(request, response);
}

_HTTPD_END_