#include <iostream>
#include <memory>
#include "utils/Server.hpp"
#include "home.hpp"

int main()
{
    boost::asio::io_context io_context;
    HttpServer server(io_context, 8000);
    Router router = {
        {"/login", new LoginServlet(server)},
        {"/register", new RegisterServlet(server)},
        {"/assets", new AssetServlet(server)},
        {"/users", new UsersServlet(server)},
        {"/addUser", new AddUserServlet(server)},
        {"/deleteUser", new DeleteUserServlet(server)},
        {"/addAsset", new AddAssetServlet(server)},
        {"/deleteAsset", new DeleteAssetServlet(server)},
        {"/changeAsset", new ChangeAssetServlet(server)},
        {"/updateAsset", new UpdateAssetServlet(server)},
        {"/logs", new LogsServlet(server)},
        {"/updateType", new UpdateTypeServlet(server)},
        {"/updateCode", new UpdateCodeServlet(server)},
        {"/Types", new TypeServlet(server)},
    };
    server.router(router);
    server.database("localhost", "root", "Czy010207...", "assets");
    server.run();
    return 0;
}