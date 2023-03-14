#pragma once

#include "Interface.hpp"
#include "Router.hpp"
#include "mysqlconn.hpp"
#include <map>

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
    std::cout << "doGet\n";
    std::string path = request.get_path();
    if (router_.count(path) == 0)
    {
        response.set_body("Not Found");
        response.set_status_code(HttpServletResponse::NOT_FOUND);
        response.send();
        return;
    }
    std::cout << "path: " << path << std::endl;
    auto servlet = (router_.getRouter(path));
    servlet->doGet(request, response);
}

void DispatcherServlet::doPost(HttpServletRequest &request, HttpServletResponse &response)
{
    std::cout << "doPost\n";
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