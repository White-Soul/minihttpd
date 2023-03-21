#pragma once
#include "./utils/Interface.hpp"
#include "./utils/Server.hpp"
#include "./utils/mysqlconn.hpp"
#include "./utils/utils.hpp"
#include "./custom/custom.hpp"
#include <boost/json.hpp>

// 登录
class LoginServlet : public HttpServlet
{
public:
    LoginServlet(HttpServer &ser) : HttpServlet(ser) {}
    void doGet(HttpServletRequest &request, HttpServletResponse &response)
    {
    }
    void doPost(HttpServletRequest &request, HttpServletResponse &response)
    {
        auto json = request.get_body();
        auto data_obj = body_to_obj(json);
        std::string token = generate_token();
        std::string account = getObjValue<std::string>(data_obj, "account");
        std::string password = getObjValue<std::string>(data_obj, "password");
        auto sql = "select * from user where account = '" + account + "' and password = '" + password + "';";
        auto conn = this->get_server().getConnPool()->getConnection();
        Result res;
        try
        {
            auto result = conn->sql(sql).execute();
            if (result.count())
            {
                User u = result_to_obj<User>(result);
                res.set_Data(u.serialize());
                res.set_Result_Code(Result_Code::OK_);
                res.set_Message("存在用户");
                save_token(conn, token, u);
            }
            else
            {
                res.set_Result_Code(Result_Code::WARN_);
                res.set_Message("不存在用户/密码错误");
            }
        }
        catch (const mysqlx::Error &e)
        {
            res.set_Result_Code(Result_Code::ERROR_);
            res.set_Message("数据库异常");
            std::cerr << "Failed to execute SQL statement: " << e.what() << std::endl;
        }
        response.set_body(result_to_json(res));
        get_server().getConnPool()->releaseConnection(conn);
        response.set_status_code(HttpServletResponse::OK);
        response.set_content_type("application/json;");
        response.set_authorization(token);
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
        auto conn = get_server().getConnPool()->getConnection();
        Result res;
        try
        {
            auto result = conn->sql(sql).execute();
            if (result.count())
            {
                res.set_Result_Code(Result_Code::WARN_);
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
        }
        catch (const mysqlx::Error &e)
        {
            res.set_Result_Code(Result_Code::ERROR_);
            res.set_Message("SQL执行错误");
            std::cerr << "Failed to execute SQL statement: " << e.what() << std::endl;
        }
        response.set_body(result_to_json(res));
        get_server().getConnPool()->releaseConnection(conn);
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
    }
    void doPost(HttpServletRequest &request, HttpServletResponse &response) override
    {
        User u = json_to_obj<User>(request.get_body());
        auto conn = get_server().getConnPool()->getConnection();
        Result res;
        try
        {
            mysqlx::abi2::r0::RowResult result;
            if (u.isRoot)
            {
                result = conn->getDefaultSchema().getTable("asset").select("*").execute();
            }
            else
            {
                result = conn->getDefaultSchema().getTable("asset").select("*").where("user_account = '" + u.account + "'").execute();
            }
            res.set_Result_Code(Result_Code::OK_);
            res.set_Message("查询成功");
            res.set_Data(rowResult_to_json(result));
        }
        catch (const mysqlx::Error &e)
        {
            res.set_Result_Code(Result_Code::ERROR_);
            res.set_Message("SQL执行错误");
            std::cerr << "Failed to execute SQL statement: " << e.what() << std::endl;
        }

        get_server().getConnPool()->releaseConnection(conn);
        response.set_status_code(HttpServletResponse::OK);
        response.set_content_type("application/json");
        response.set_body(result_to_json(res));
        response.send();
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
        auto conn = get_server().getConnPool()->getConnection();
        Result res;
        try
        {
            auto result = conn->getDefaultSchema().getTable("user").select("*").execute();
            std::string body = rowResult_to_json(result);
            res.set_Result_Code(Result_Code::OK_);
            res.set_Message("请求成功");
            res.set_Data(body);
        }
        catch (const mysqlx::Error &e)
        {
            res.set_Result_Code(Result_Code::ERROR_);
            res.set_Message("SQL执行错误");
            std::cerr << "Failed to execute SQL statement: " << e.what() << std::endl;
        }
        get_server().getConnPool()->releaseConnection(conn);
        response.add_body(result_to_json(res));
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
        Result res;
        auto conn = get_server().getConnPool()->getConnection();
        try
        {
            auto result = conn->sql(sql).execute();
            std::string body = rowResult_to_json(result);
            res.set_Data(body);
            res.set_Result_Code(Result_Code::OK_);
            res.set_Message("请求成功");
        }
        catch (const mysqlx::Error &e)
        {
            std::cerr << "Failed to execute SQL statement: " << e.what() << std::endl;
            res.set_Result_Code(Result_Code::ERROR_);
            res.set_Message("请求失败");
        }
        get_server().getConnPool()->releaseConnection(conn);
        response.add_body(result_to_json(res));
        response.set_status_code(HttpServletResponse::OK);
        response.set_content_type("application/json");
        response.send();
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
        u.name = u.account;
        auto conn = get_server().getConnPool()->getConnection();
        std::string sql = "Insert into user(name, account, password) values('?','?','?')";
        Result res;
        try
        {
            conn->sql(sql).bind(u.name, u.account, u.password).execute();
            res.set_Message("插入成功");
            res.set_Result_Code(Result_Code::OK_);
        }
        catch (const mysqlx::Error &e)
        {
            res.set_Message("插入失败");
            res.set_Result_Code(Result_Code::ERROR_);
            std::cerr << "Failed to execute SQL statement: " << e.what() << std::endl;
        }
        get_server().getConnPool()->releaseConnection(conn);
        response.add_body(result_to_json(res));
        response.set_status_code(HttpServletResponse::OK);
        response.send();
    }

    AddUserServlet(HttpServer &ser) : HttpServlet(ser) {}
};
// 删除用户
class DeleteUserServlet : public HttpServlet
{
public:
    void doGet(HttpServletRequest &request, HttpServletResponse &response)
    {
        auto account = request.get_parameter("account");
        std::string sql = "delete from user where account = ?;";
        std::shared_ptr<mysqlx::abi2::r0::Session> conn;
        Result res;
        try
        {
            conn = get_server().getConnPool()->getConnection();
            conn->sql(sql).bind(account).execute();
            res.set_Message("删除成功");
            res.set_Result_Code(Result_Code::OK_);
        }
        catch (const mysqlx::Error e)
        {
            res.set_Message("删除失败");
            res.set_Result_Code(Result_Code::ERROR_);
            std::cerr << e.what() << std::endl;
        }
        get_server().getConnPool()->releaseConnection(conn);
        response.add_body(result_to_json(res));
        response.set_status_code(HttpServletResponse::OK);
        response.set_content_type("application/json");
        response.send();
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
        asset.code = getUUID();
        std::string sql = "insert into asset(money, amount, name, desc, code, type_id, user_account) values (?,?,?,?,?,?,?);";
        auto conn = get_server().getConnPool()->getConnection();
        Result res;
        try
        {
            conn->sql(sql).bind(asset.money, asset.amount, asset.name, asset.desc, asset.code, asset.type_id, asset.user_account).execute();
            res.set_Result_Code(Result_Code::OK_);
            res.set_Message("入库成功");
        }
        catch (const mysqlx::Error &e)
        {
            res.set_Result_Code(Result_Code::ERROR_);
            res.set_Message("入库失败");
            std::cerr << "Failed to execute SQL statement: " << e.what() << std::endl;
        }
        get_server().getConnPool()->releaseConnection(conn);
        response.set_status_code(HttpServletResponse::OK);
        response.add_body(result_to_json(res));
        response.send();
    }

    AddAssetServlet(HttpServer &ser) : HttpServlet(ser) {}
};
// 删除资产
class DeleteAssetServlet : public HttpServlet
{
public:
    void doGet(HttpServletRequest &request, HttpServletResponse &response)
    {
        auto code = request.get_parameter("code");
        std::string sql = "delete from asset where code = ?";
        auto conn = get_server().getConnPool()->getConnection();
        Result res;
        try
        {
            conn->sql(sql).bind(code).execute();
            res.set_Message("删除成功");
            res.set_Result_Code(Result_Code::OK_);
        }
        catch (const mysqlx::Error &e)
        {
            res.set_Message("删除失败");
            res.set_Result_Code(Result_Code::ERROR_);
            std::cout << "SQL ERROR: " << e.what() << std::endl;
        }
        get_server().getConnPool()->releaseConnection(conn);
        response.set_status_code(HttpServletResponse::OK);
        response.set_content_type("text/html");
        response.send();
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
        auto conn = get_server().getConnPool()->getConnection();
        Result res;
        std::string sql = "update asset set user_account = ? where user_account = ? and code = ?";
        try
        {
            for (auto a : selected)
            {
                conn->sql(sql).bind(toUser, fromUser, a).execute();
            }
            res.set_Result_Code(Result_Code::OK_);
            res.set_Message("更新成功");
        }
        catch (const mysqlx::Error &e)
        {
            res.set_Result_Code(Result_Code::ERROR_);
            res.set_Message("更新失败");
            std::cerr << "Failed to execute SQL statement: " << e.what() << std::endl;
        }
        get_server().getConnPool()->releaseConnection(conn);
        response.set_status_code(HttpServletResponse::OK);
        response.add_body(result_to_json(res));
        response.send();
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
        auto body = request.get_body();
        Asset ass = json_to_obj<Asset>(body);
        auto conn = get_server().getConnPool()->getConnection();
        Result res;
        try
        {
            std::string sql = "update asset set type_id=?,name=?,money=?,amount=? where code = ?";
            conn->sql(sql).bind(ass.type_id, ass.name, ass.money, ass.amount, ass.code).execute();
            res.set_Message("更新成功");
            res.set_Result_Code(Result_Code::OK_);
        }
        catch (const mysqlx::Error &e)
        {
            std::cerr << "SQL Error: " << e.what() << std::endl;
            res.set_Message("更新失败");
            res.set_Result_Code(Result_Code::ERROR_);
        }
        get_server().getConnPool()->releaseConnection(conn);
        response.set_status_code(HttpResponse::OK);
        response.set_content_type("text/html");
        response.send();
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
        auto body = request.get_body();
        Asset t = json_to_obj<Asset>(body);
        std::string sql = "update asset set type_id = ? where code = ?";
        auto conn = get_server().getConnPool()->getConnection();
        Result res;
        try
        {
            conn->sql(sql).bind(t.type_id, t.code).execute();
            res.set_Message("更新类型成功");
            res.set_Result_Code(Result_Code::OK_);
        }
        catch (const mysqlx::Error &e)
        {
            res.set_Message("更新类型失败");
            res.set_Result_Code(Result_Code::ERROR_);
            std::cerr << e.what() << std::endl;
        }
        get_server().getConnPool()->releaseConnection(conn);
        response.set_status_code(HttpServletResponse::OK);
        response.send();
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
        std::string newCode = request.get_parameter("newCode");
        std::string oldCode = request.get_parameter("oldCode");
        std::string sql = "update asset set code = ? where code = ?";
        auto conn = get_server().getConnPool()->getConnection();
        Result res;
        try
        {
            conn->sql(sql).bind(newCode, oldCode).execute();
            res.set_Message("Code更新成功");
            res.set_Result_Code(Result_Code::OK_);
        }
        catch (const mysqlx::Error &e)
        {
            std::cerr << "SQL Error: " << e.what() << std::endl;
            res.set_Message("Code更新失败");
            res.set_Result_Code(Result_Code::ERROR_);
        }
        get_server().getConnPool()->releaseConnection(conn);
        response.set_status_code(HttpServletResponse::OK);
        response.set_content_type("text/html");
        response.send();
    }
    UpdateCodeServlet(HttpServer &ser) : HttpServlet(ser) {}
};
// 获取类型
class TypeServlet : public HttpServlet
{
public:
    void doGet(HttpServletRequest &request, HttpServletResponse &response)
    {
        std::string sql = "select * from type";
        auto conn = get_server().getConnPool()->getConnection();
        Result result;
        try
        {
            auto res = conn->sql(sql).execute();
            result.set_Data(rowResult_to_json(res));
            result.set_Result_Code(Result_Code::OK_);
            result.set_Message("请求成功");
            response.add_body(result_to_json(result));
        }
        catch (const mysqlx::Error &e)
        {
            result.set_Result_Code(Result_Code::ERROR_);
            result.set_Message("请求失败");
            std::cerr << "Failed to execute SQL statement: " << e.what() << std::endl;
        }
        get_server().getConnPool()->releaseConnection(conn);
        response.set_status_code(HttpServletResponse::OK);
        response.set_content_type("application/json");
        response.send();
    }
    void doPost(HttpServletRequest &request, HttpServletResponse &response)
    {
    }

    TypeServlet(HttpServer &ser) : HttpServlet(ser) {}
};
