#pragma once
#include "./utils/Interface.hpp"
#include "./utils/Server.hpp"
#include "./utils/mysqlconn.hpp"
#include "./utils/utils.hpp"
#include "./custom/custom.hpp"
#include <boost/json.hpp>

class homeservlet : public HttpServlet
{
public:
    virtual void doGet(HttpServletRequest &request, HttpServletResponse &response)
    {
        auto conn = ConnectionPool("localhost", "root", "Czy010207...", "test").getConnection();
        auto result = conn->getDefaultSchema().getTable("test").select("*").execute();
        auto json_str = rowResult_to_json(result);
        response.set_content_type("application/json;");
        response.add_body(json_str);
        response.set_status_code(HttpServletResponse::OK);
        response.send();
        std::cout << "get......home" << std::endl;
    }
    virtual void doPost(HttpServletRequest &request, HttpServletResponse &response)
    {
        doGet(request, response);
    }
};
// 登录
class LoginServlet : public HttpServlet
{
public:
    LoginServlet(HttpServer &ser) : HttpServlet(ser) {}
    LoginServlet() = default;
    void doGet(HttpServletRequest &request, HttpServletResponse &response)
    {
    }
    void doPost(HttpServletRequest &request, HttpServletResponse &response)
    {
        auto json = request.get_body();
        auto data_obj = body_to_obj(json);
        std::string account = getObjValue<std::string>(data_obj, "account");
        std::string password = getObjValue<std::string>(data_obj, "password");
        auto sql = "select * from user where account = '" + account + "' and password = '" + password + "';";
        try
        {
            auto conn = server_.getConnPool()->getConnection();
            auto result = conn->sql(sql).execute();
            Result res;
            if (result.count())
            {
                User u = result_to_obj<User>(result);
                if (u.password == password)
                {
                    res.set_Data(u.serialize());
                    res.set_Result_Code(Result_Code::OK_);
                    res.set_Message("存在用户");
                }
                else
                {
                    res.set_Result_Code(Result_Code::WARN_);
                    res.set_Message("密码错误");
                }
            }
            else
            {
                res.set_Result_Code(Result_Code::ERROR_);
                res.set_Message("不存在用户");
            }
            response.set_body(result_to_json(res));
        }
        catch (const mysqlx::Error &e)
        {
            std::cerr << "Failed to execute SQL statement: " << e.what() << std::endl;
        }
        response.set_status_code(HttpServletResponse::OK);
        response.set_content_type("application/json;");
        response.send();
    }
};
// 注册
class RegisterServlet : public HttpServlet
{
public:
    RegisterServlet() = default;
    RegisterServlet(HttpServer &ser) : HttpServlet(ser) {}
    void doGet(HttpServletRequest &request, HttpServletResponse &response)
    {
    }
    void doPost(HttpServletRequest &request, HttpServletResponse &response)
    {

        auto j = request.get_body();
        auto data_obj = body_to_obj(j);
        User u;
        u.account = getObjValue<std::string>(data_obj, "account");
        u.password = getObjValue<std::string>(data_obj, "password");
        u.email = getObjValue<std::string>(data_obj, "email");
        // 获取当前系统时钟时间
        auto now = std::chrono::system_clock::now();
        // 转换为时间戳
        auto now_c = std::chrono::system_clock::to_time_t(now);
        // u.id = now_c;
        auto sql = "select * from user where name = " + u.name;
        try
        {
            auto conn = server_.getConnPool()->getConnection();
            auto result = conn->sql(sql).execute();
            Result res;
            if (result.count())
            {
                res.set_Result_Code(Result_Code::ERROR_);
                res.set_Message("存在用户");
            }
            else
            {
                std::string sql = "INSERT INTO user ( name, account, password, isRoot, dept_id) VALUES (" +
                                  u.name + "', '" + u.account + "', '" + u.password + "', " +
                                  std::to_string(u.isRoot) + ")";
                conn->sql(sql).execute();
                res.set_Result_Code(Result_Code::OK_);
                res.set_Message("插入成功");
            }
            response.set_body(result_to_json(res));
        }
        catch (const mysqlx::Error &e)
        {
            std::cerr << "Failed to execute SQL statement: " << e.what() << std::endl;
        }

        response.set_status_code(HttpServletResponse::OK);
        response.set_content_type("text/html");
        response.send();
    }
};
// 获取资产列表
class AssetServlet : public HttpServlet
{
public:
    void doGet(HttpServletRequest &request, HttpServletResponse &response) override
    {
        User u = json_to_obj<User>(request.get_body());
        auto conn = server_.getConnPool()->getConnection();
        mysqlx::abi2::r0::RowResult result;
        try
        {
            if (u.isRoot)
            {
                result = conn->getDefaultSchema().getTable("asset").select("*").execute();
            }
            else
            {
                result = conn->getDefaultSchema().getTable("asset").select("*").where("user_account = " + u.account).execute();
            }
        }
        catch (const mysqlx::Error &e)
        {
            std::cerr << "Failed to execute SQL statement: " << e.what() << std::endl;
        }
        Result res;
        res.set_Result_Code(Result_Code::OK_);
        res.set_Message("查询成功");
        res.set_Data(rowResult_to_json(result));
        response.set_status_code(HttpServletResponse::OK);
        response.set_content_type("application/json");
        response.set_body(result_to_json(res));
        response.send();
    }
    void doPost(HttpServletRequest &request, HttpServletResponse &response) override
    {
    }
    ~AssetServlet() override = default;
    AssetServlet(HttpServer &ser) : HttpServlet(ser) {}
};
// 获取用户列表
class UsersServlet : public HttpServlet
{
public:
    void doGet(HttpServletRequest &request, HttpServletResponse &response)
    {
        auto conn = server_.getConnPool()->getConnection();
        try
        {
            auto result = conn->getDefaultSchema().getTable("user").select("*").execute();
            std::string body = rowResult_to_json(result);
            response.add_body(body);
        }
        catch (const mysqlx::Error &e)
        {
            std::cerr << "Failed to execute SQL statement: " << e.what() << std::endl;
        }
        response.set_status_code(HttpServletResponse::OK);
        response.set_content_type("application/json");
        response.send();
    }
    void doPost(HttpServletRequest &request, HttpServletResponse &response)
    {
    }

    UsersServlet(HttpServer &ser) : HttpServlet(ser) {}
};
// 获取某个用户的资产列表
class UserAssetServlet : public HttpServlet
{
public:
    void doGet(HttpServletRequest &request, HttpServletResponse &response)
    {
    }
    void doPost(HttpServletRequest &request, HttpServletResponse &response)
    {
        auto body = request.get_body();
        User u = json_to_obj<User>(body);
        std::string sql = "select * from asset where (user_account = '" + u.account + "');";
    }
};
// 添加用户
class AddUserServlet : public HttpServlet
{
public:
    void doGet(HttpServletRequest &request, HttpServletResponse &response)
    {
    }
    void doPost(HttpServletRequest &request, HttpServletResponse &response)
    {
        auto body = request.get_body();
        User u = json_to_obj<User>(body);
        auto conn = server_.getConnPool()->getConnection();
        std::string sql = "Insert into user(name, account, password) values('?','?','?')";
        try
        {
            conn->sql(sql).bind(u.name, u.account, u.password).execute();
        }
        catch (const mysqlx::Error &e)
        {
            std::cerr << "Failed to execute SQL statement: " << e.what() << std::endl;
        }
    }

    AddUserServlet(HttpServer &ser) : HttpServlet(ser) {}
};
// 删除用户
class DeleteUserServlet : public HttpServlet
{
public:
    void doGet(HttpServletRequest &request, HttpServletResponse &response)
    {
    }
    void doPost(HttpServletRequest &request, HttpServletResponse &response)
    {
    }

    DeleteUserServlet(HttpServer &ser) : HttpServlet(ser) {}
};
// 添加资产
class AddAssetServlet : public HttpServlet
{
public:
    void doGet(HttpServletRequest &request, HttpServletResponse &response)
    {
    }
    void doPost(HttpServletRequest &request, HttpServletResponse &response)
    {
        auto body = request.get_body();
        Asset asset = json_to_obj<Asset>(body);
        std::string sql = "insert into asset(money, amount, name, desc, code, type_id, user_account) values (" + std::to_string(asset.money) +
                          "," + std::to_string(asset.amount) +
                          ",'" + asset.name + "', '" + asset.desc +
                          "','" + asset.code + "','" + asset.type_id +
                          "','" + asset.user_account + "');";
        try
        {
            auto conn = server_.getConnPool()->getConnection();
            conn->sql(sql).execute();
        }
        catch (const mysqlx::Error &e)
        {
            std::cerr << "Failed to execute SQL statement: " << e.what() << std::endl;
        }
    }

    AddAssetServlet(HttpServer &ser) : HttpServlet(ser) {}
};
// 删除资产
class DeleteAssetServlet : public HttpServlet
{
public:
    void doGet(HttpServletRequest &request, HttpServletResponse &response)
    {
    }
    void doPost(HttpServletRequest &request, HttpServletResponse &response)
    {
    }

    DeleteAssetServlet(HttpServer &ser) : HttpServlet(ser) {}
};
// 资产变更
class ChangeAssetServlet : public HttpServlet
{
public:
    void doGet(HttpServletRequest &request, HttpServletResponse &response)
    {
    }
    void doPost(HttpServletRequest &request, HttpServletResponse &response)
    {
        std::string body = request.get_body();
        auto obj = body_to_obj(body);
        std::string fromUser = getObjValue<std::string>(obj, "formUser");
        std::string toUser = getObjValue<std::string>(obj, "toUser");
        std::vector<std::string> selected = getArrayValue<std::string>(obj, "selected");
        try
        {
            auto conn = server_.getConnPool()->getConnection();
            for (auto a : selected)
            {
                conn->getDefaultSchema().getTable("asset").update().set("user_account", toUser).where("code = " + a);
            }
        }
        catch (const mysqlx::Error &e)
        {
            std::cerr << "Failed to execute SQL statement: " << e.what() << std::endl;
        }
    }
    ChangeAssetServlet(HttpServer &ser) : HttpServlet(ser) {}
};
// 更新资产
class UpdateAssetServlet : public HttpServlet
{
public:
    void doGet(HttpServletRequest &request, HttpServletResponse &response)
    {
    }
    void doPost(HttpServletRequest &request, HttpServletResponse &response)
    {
    }
    UpdateAssetServlet(HttpServer &ser) : HttpServlet(ser) {}
};
// 日志
class LogsServlet : public HttpServlet
{
public:
    void doGet(HttpServletRequest &request, HttpServletResponse &response)
    {
    }
    void doPost(HttpServletRequest &request, HttpServletResponse &response)
    {
    }

    LogsServlet(HttpServer &ser) : HttpServlet(ser) {}
};
// 更新类型
class UpdateTypeServlet : public HttpServlet
{
public:
    void doGet(HttpServletRequest &request, HttpServletResponse &response)
    {
    }
    void doPost(HttpServletRequest &request, HttpServletResponse &response)
    {
    }
    UpdateTypeServlet(HttpServer &ser) : HttpServlet(ser) {}
};
// 更新编码
class UpdateCodeServlet : public HttpServlet
{
public:
    void doGet(HttpServletRequest &request, HttpServletResponse &response)
    {
    }
    void doPost(HttpServletRequest &request, HttpServletResponse &response)
    {
    }
    UpdateCodeServlet(HttpServer &ser) : HttpServlet(ser) {}
};
