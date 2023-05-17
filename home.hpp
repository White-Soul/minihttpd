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
                save_token(conn, token, u.account);
                get_server().getTimer().SetInterval(60 * 60,
                                                    std::bind(&delete_token, conn, token));
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
            HttpdLog::Error("Failed to execute SQL statement: "+std::string(e.what()), (char*)__FUNCTIONW__);
        }
        response.set_body(result_to_json(res));
        get_server().getConnPool()->releaseConnection(conn);
        response.set_status_code(HttpServletResponse::OK);
        response.set_content_type("application/json;");
        response.set_authorization(token);
        response.send();
    }
};
// 退出登录
class LogoutServlet : public HttpServlet
{
public:
    LogoutServlet(HttpServer &ser) : HttpServlet(ser) {}
    void doGet(HttpServletRequest &request, HttpServletResponse &response)
    {
        auto conn = get_server().getConnPool()->getConnection();
        delete_token(conn, request.get_authorization());
        get_server().getConnPool()->releaseConnection(conn);
        response.set_status_code(HttpResponse::OK);
        response.set_content_type("text/html");
        response.send();
    }
    void doPost(HttpServletRequest &request, HttpServletResponse &response)
    {
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
            HttpdLog::Error("Failed to execute SQL statement: "+std::string(e.what()), (char*)__FUNCTIONW__);
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
        auto conn = get_server().getConnPool()->getConnection();
        if (get_token(conn, request.get_authorization()))
        {

            User u = json_to_obj<User>(request.get_body());

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
                HttpdLog::Error("Failed to execute SQL statement: "+std::string(e.what()), (char*)__FUNCTIONW__);
            }

            get_server().getConnPool()->releaseConnection(conn);
            response.set_status_code(HttpServletResponse::OK);
            response.set_content_type("application/json");
            response.set_body(result_to_json(res));
        }
        else
        {
            response.set_status_code(HttpServletResponse::NOT_FOUND);
            response.set_content_type("text/html");
        }
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
        if (get_token(conn, request.get_authorization()))
        {

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
                HttpdLog::Error("Failed to execute SQL statement: "+std::string(e.what()), (char*)__FUNCTIONW__);
            }
            response.add_body(result_to_json(res));
            response.set_status_code(HttpServletResponse::OK);
            response.set_content_type("application/json");
        }
        else
        {
            response.set_status_code(HttpServletResponse::NOT_FOUND);
            response.set_content_type("text/html");
        }
        get_server().getConnPool()->releaseConnection(conn);
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
        auto conn = get_server().getConnPool()->getConnection();
        if (get_token(conn, request.get_authorization()))
        {

            auto body = request.get_body();
            User u = json_to_obj<User>(body);
            std::string sql = "select * from asset where (user_account = '" + u.account + "');";
            Result res;

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
                HttpdLog::Error("Failed to execute SQL statement: "+std::string(e.what()), (char*)__FUNCTIONW__);
                res.set_Result_Code(Result_Code::ERROR_);
                res.set_Message("请求失败");
            }
            response.add_body(result_to_json(res));
            response.set_status_code(HttpServletResponse::OK);
            response.set_content_type("application/json");
        }
        else
        {
            response.set_status_code(HttpServletResponse::NOT_FOUND);
            response.set_content_type("text/html");
        }
        get_server().getConnPool()->releaseConnection(conn);
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
        auto conn = get_server().getConnPool()->getConnection();
        if (get_token(conn, request.get_authorization()))
        {
            auto body = request.get_body();
            User u = json_to_obj<User>(body);
            u.name = u.account;
            std::string sql = "Insert into user(name, account, password) values(?,?,?)";
            Result res;
            try
            {
                conn->sql(sql).bind(u.name, u.account, u.password).execute();
                res.set_Message("插入成功");
                res.set_Result_Code(Result_Code::OK_);
            }
            catch (const mysqlx::Error &e)
            {
                res.set_Message("插入失败,用户或密码有错误");
                res.set_Result_Code(Result_Code::ERROR_);
                HttpdLog::Error("Failed to execute SQL statement: "+std::string(e.what()), (char*)__FUNCTIONW__);
            }
            get_server().getConnPool()->releaseConnection(conn);
            response.add_body(result_to_json(res));
            response.set_status_code(HttpServletResponse::OK);
        }
        else
        {
            response.set_status_code(HttpServletResponse::NOT_FOUND);
            response.set_content_type("text/html");
        }
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
        auto conn = get_server().getConnPool()->getConnection();
        if (get_token(conn, request.get_authorization()))
        {
            auto account = request.get_parameter("account");
            std::string sql = "delete from user where account = ?;";
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
                HttpdLog::Error("Failed to execute SQL statement: "+std::string(e.what()), (char*)__FUNCTIONW__);
            }
            get_server().getConnPool()->releaseConnection(conn);
            response.add_body(result_to_json(res));
            response.set_status_code(HttpServletResponse::OK);
            response.set_content_type("application/json");
        }
        else
        {
            response.set_status_code(HttpServletResponse::NOT_FOUND);
            response.set_content_type("text/html");
        }
        response.send();
    }
    void doPost(HttpServletRequest &request, HttpServletResponse &response)
    {
    }

    DeleteUserServlet(HttpServer &ser) : HttpServlet(ser) {}
};
// 更新用户
class UpdateUser : public HttpServlet
{
public:
    UpdateUser(HttpServer &ser) : HttpServlet(ser) {}
    void doGet(HttpServletRequest &req, HttpServletResponse &resp) override
    {
    }
    void doPost(HttpServletRequest &req, HttpServletResponse &resp) override
    {
        auto conn = get_server().getConnPool()->getConnection();
        try
        {
            auto body = req.get_body();
            User u = json_to_obj<User>(body);
            std::string sql = "update user set "
             "password = ?,name=?,email=?,phone=?,address=?,qq=?,wchat=?"
             "where account = ?";
            conn->sql(sql).bind(u.password,u.name,u.email,u.phone,u.address,u.qq,u.wchat, u.account).execute();
            resp.set_status_code(HttpResponse::OK);
            resp.set_content_type("text/html");
        }
        catch (...)
        {
            handle_excepiton(std::current_exception());
            resp.set_status_code(HttpResponse::INTERNAL_SERVER_ERROR);
            resp.set_content_type("text/html");
        }
        resp.send();
    }
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
        auto conn = get_server().getConnPool()->getConnection();
        if (get_token(conn, request.get_authorization()))
        {

            auto body = request.get_body();
            Asset asset = json_to_obj<Asset>(body);
            asset.code = getUUID();
            bool flag = asset.desc == "" ? true : false;
            std::string sql;
            if(flag)
                sql = "insert into asset(money, amount, name, desc, code, type_id, user_account) values (?,?,?,?,?,?,?)";
            else
                sql = "insert into asset(money, amount, name, code, type_id, user_account) values (?,?,?,?,?,?)";

            Result res;
            try
            {
                if(flag)
                    conn->sql(sql).bind(asset.money, asset.amount, asset.name, asset.code, asset.type_id, asset.user_account).execute();
                else
                    conn->sql(sql).bind(asset.money, asset.amount, asset.name, asset.desc, asset.code, asset.type_id, asset.user_account).execute();
                res.set_Result_Code(Result_Code::OK_);
                res.set_Message("入库成功");
            }
            catch (const mysqlx::Error &e)
            {
                res.set_Result_Code(Result_Code::ERROR_);
                res.set_Message("入库失败");
                HttpdLog::Error("Failed to execute SQL statement: "+std::string(e.what()), (char*)__FUNCTIONW__);
            }
            get_server().getConnPool()->releaseConnection(conn);
            response.set_status_code(HttpServletResponse::OK);
            response.add_body(result_to_json(res));
        }
        else
        {
            response.set_status_code(HttpServletResponse::NOT_FOUND);
            response.set_content_type("text/html");
        }
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
        auto conn = get_server().getConnPool()->getConnection();
        if (get_token(conn, request.get_authorization()))
        {

            auto code = request.get_parameter("code");
            std::string sql = "delete from asset where code = ?";

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
                HttpdLog::Error("Failed to execute SQL statement: "+std::string(std::string(e.what())), (char*)__FUNCTIONW__);
            }
            get_server().getConnPool()->releaseConnection(conn);
            response.set_status_code(HttpServletResponse::OK);
        }
        else
        {
            response.set_status_code(HttpServletResponse::NOT_FOUND);
        }
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
        auto conn = get_server().getConnPool()->getConnection();
        if (get_token(conn, request.get_authorization()))
        {

            std::string body = request.get_body();
            auto obj = body_to_obj(body);
            std::string fromUser = getObjValue<std::string>(obj, "formUser");
            std::string toUser = getObjValue<std::string>(obj, "toUser");
            std::vector<std::string> selected = getArrayValue<std::string>(obj, "selected");

            Result res;
            std::string sql = "update asset set user_account = ? where code = ?";
            try
            {
                for (auto a : selected)
                {
                    conn->sql(sql).bind(toUser, a).execute();
                }
                res.set_Result_Code(Result_Code::OK_);
                res.set_Message("更新成功");
            }
            catch (const mysqlx::Error &e)
            {
                res.set_Result_Code(Result_Code::ERROR_);
                res.set_Message("更新失败");
                HttpdLog::Error("Failed to execute SQL statement: " + std::string(std::string(e.what())), (char*)__FUNCTIONW__);
            }
            get_server().getConnPool()->releaseConnection(conn);
            response.set_status_code(HttpServletResponse::OK);
            response.add_body(result_to_json(res));
        }
        else
        {
            response.set_status_code(HttpServletResponse::NOT_FOUND);
            response.set_content_type("text/html");
        }
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
        auto conn = get_server().getConnPool()->getConnection();
        if (get_token(conn, request.get_authorization()))
        {

            auto body = request.get_body();
            Asset ass = json_to_obj<Asset>(body);

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
                std::cerr << "SQL Error: " << std::string(e.what()) << std::endl;
                res.set_Message("更新失败");
                res.set_Result_Code(Result_Code::ERROR_);
            }
            get_server().getConnPool()->releaseConnection(conn);
            response.set_status_code(HttpResponse::OK);
        }
        else
        {
            response.set_status_code(HttpResponse::NOT_FOUND);
        }
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
        auto conn = get_server().getConnPool()->getConnection();
        if (get_token(conn, request.get_authorization()))
        {
            Result res;
            std::string sql = "select * from log";
            try
            {
                auto result = conn->sql(sql).execute();
                res.set_Data(rowResult_to_json(result));
                res.set_Result_Code(Result_Code::OK_);
                res.set_Message("查询成功");
            }
            catch (const mysqlx::Error &e)
            {
                res.set_Message("查询失败");
                res.set_Result_Code(Result_Code::ERROR_);
                HttpdLog::Error(std::string(e.what()),(char*)__FUNCTIONW__);
            }
            get_server().getConnPool()->releaseConnection(conn);
            response.set_body(result_to_json(res));
            response.set_status_code(HttpResponse::OK);
            response.set_content_type("application/json");
        }
        else
        {
            response.set_status_code(HttpResponse::NOT_FOUND);
            response.set_content_type("text/html");
        }
        response.send();
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
        auto conn = get_server().getConnPool()->getConnection();
        if (get_token(conn, request.get_authorization()))
        {

            auto body = request.get_body();
            Asset t = json_to_obj<Asset>(body);
            std::string sql = "update asset set type_id = ? where code = ?";

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
                std::cerr << std::string(e.what()) << std::endl;
            }
            get_server().getConnPool()->releaseConnection(conn);
            response.set_status_code(HttpServletResponse::OK);
        }
        else
        {
            response.set_status_code(HttpServletResponse::NOT_FOUND);
            response.set_content_type("text/html");
        }
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
        auto conn = get_server().getConnPool()->getConnection();
        if (get_token(conn, request.get_authorization()))
        {

            std::string newCode = request.get_parameter("newCode");
            std::string oldCode = request.get_parameter("oldCode");
            std::string sql = "update asset set code = ? where code = ?";

            Result res;
            try
            {
                conn->sql(sql).bind(newCode, oldCode).execute();
                res.set_Message("Code更新成功");
                res.set_Result_Code(Result_Code::OK_);
            }
            catch (const mysqlx::Error &e)
            {
                std::cerr << "SQL Error: " << std::string(e.what()) << std::endl;
                res.set_Message("Code更新失败");
                res.set_Result_Code(Result_Code::ERROR_);
            }
            get_server().getConnPool()->releaseConnection(conn);
            response.set_status_code(HttpServletResponse::OK);
        }
        else
        {
            response.set_status_code(HttpServletResponse::NOT_FOUND);
        }
        response.set_content_type("text/html");
        response.send();
    }
    void doPost(HttpServletRequest &request, HttpServletResponse &response)
    {
        
    }
    UpdateCodeServlet(HttpServer &ser) : HttpServlet(ser) {}
};
// 获取类型
class TypeServlet : public HttpServlet
{
public:
    void doGet(HttpServletRequest &request, HttpServletResponse &response) override
    {
        auto conn = get_server().getConnPool()->getConnection();
        if (get_token(conn, request.get_authorization()))
        {

            std::string sql = "select * from type";

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
                std::cerr << "Failed to execute SQL statement: " << std::string(e.what()) << std::endl;
            }
            get_server().getConnPool()->releaseConnection(conn);
            response.set_status_code(HttpServletResponse::OK);
            response.set_content_type("application/json");
        }
        else
        {
            response.set_status_code(HttpServletResponse::NOT_FOUND);
            response.set_content_type("text/html");
        }
        response.send();
    }
    void doPost(HttpServletRequest &request, HttpServletResponse &response)
    {
    }

    TypeServlet(HttpServer &ser) : HttpServlet(ser) {}
};
// 保存日志
class SaveLog : public HttpServlet
{
public:
    SaveLog(HttpServer &ser) : HttpServlet(ser) {}
    void doGet(HttpServletRequest &req, HttpServletResponse &resp)
    {
    }
    void doPost(HttpServletRequest &req, HttpServletResponse &resp) override
    {

        auto conn = get_server().getConnPool()->getConnection();
        try
        {
            std::string sql = "insert into log(message, date) values(?,?)";
            auto log = req.get_body();
            Log l = json_to_obj<Log>(log);
            //LocalTime("%F")
            conn->sql(sql).bind(l.message, l.date).execute();
        }
        catch (...)
        {
            handle_excepiton(std::current_exception());
        }
        get_server().getConnPool()->releaseConnection(conn);
        resp.set_status_code(HttpResponse::OK);
        resp.set_content_type("text/html");
        resp.send();
    }
};
