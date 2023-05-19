#pragma once
#include <string>
#include <sstream>
#include <ctime>
#include <vector>
#include <mysqlx/xdevapi.h>
#include <boost/json.hpp>
#include <boost/json/src.hpp>
#include <type_traits>
#include <boost/random.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/hex.hpp>
#include "Result.hpp"
#include "Interface.hpp"
#include "Except.hpp"
#include "Logs.hpp"

_HTTPD_BEGIN_
/**
 * 将 mysqlx::Value 转为 string
 */
std::string value_to_string(const mysqlx::Value &value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}
/**
 * 将一组RowResult转为 json
 */
std::string rowResult_to_json(mysqlx::RowResult &result)
{
    boost::json::array rows;
    // Iterate over the result set and convert each row to a JSON object
    for (auto row : result.fetchAll())
    {
        boost::json::object obj;
        for (int i = 0; i < row.colCount(); ++i)
        {
            obj[std::string(result.getColumn(i).getColumnName())] = value_to_string(row.get(i));
        }
        rows.push_back(std::move(obj));
    }
    // Convert the array of rows to a JSON value
    boost::json::value val = rows;
    // Serialize the value to a JSON string
    std::string json_str = boost::json::serialize(val);
    // Print the JSON string
    return json_str;
}
/**
 * 将一个 RowResult结果转为json
 */
std::string rowResult_to_json_one(mysqlx::RowResult &result)
{
    auto row = result.fetchOne();
    boost::json::object obj;
    for (int i = 0; i < row.colCount(); ++i)
    {
        obj[std::string(result.getColumn(i).getColumnName())] = value_to_string(row.get(i));
    }
    return boost::json::serialize(obj);
}
/**
 * 将 Result 转换为 json
 */
std::string result_to_json(Result &result)
{
    boost::json::object obj;
    obj["Code"] = result.get_Result_Code_str();
    obj["Message"] = result.get_Message();
    obj["data"] = result.get_Data();

    return boost::json::serialize(obj);
}
/**
 * 将 Result 转换为 json
 */
std::string result_to_json(Result &res1, mysqlx::RowResult &res2)
{
    res1.set_Data(rowResult_to_json(res2));
    return result_to_json(res1);
}
/**
 * 将一个json字符串转换为obj
 * T 必须为 Coutom<T> 的子类
 */
template <class T>
T json_to_obj(const std::string &j)
{
    static_assert(std::is_base_of<Custom<T>, T>::value, "T must be derived from Custom<T>");
    return T::deserialize(j);
}
/**
 * 将 mysqlx::RowResult 结果集转换为 T
 *  注意只能转换第一个row
 */
template <class T>
T result_to_obj(mysqlx::RowResult &res)
{
    static_assert(std::is_base_of<Custom<T>, T>::value, "T must be derived from Custom<T>");
    return T::deserialize(rowResult_to_json_one(res));
}
/**
 * 将类型T转换为json格式，
 * T必须是Custom<T>的子类
 */
template <class T>
std::string obj_to_json(T &obj)
{
    static_assert(std::is_base_of<Custom<T>, T>::value, "T must be derived from Custom<T>");
    return obj.serialize();
}
/**
 * 获取tree树中的节点，若为空则返回对应的T();
 * T类型必须有默认构造函数
 */
template <class T>
T getPtreeNode(const boost::property_tree::ptree &tree, const std::string &str)
{
    boost::optional<const boost::property_tree::ptree &> child = tree.get_child_optional(str);
    if (child)
    {
        return child->get_value<T>();
    }
    else
    {
        return T();
    }
}
/**
 * 将 request body的内容转为 boost::json::object 对象
 */
boost::json::object body_to_obj(const std::string &body)
{
    return boost::json::parse(body).as_object();
}
/**
 * 获取 boost::json::object 的内容
 * 若失败则返回T的默认构造对象
 */
// 定义访问 string 类型成员的模板函数
template <typename T>
typename std::enable_if<std::is_same<T, std::string>::value, T>::type
get_member_impl(const boost::json::object &obj, const std::string &key)
{
    return std::string(obj.at(key).as_string().data(), obj.at(key).as_string().size());
}

// 定义访问 int64_t 类型成员的模板函数
template <typename T>
typename std::enable_if<std::is_same<T, int64_t>::value, T>::type
get_member_impl(const boost::json::object &obj, const std::string &key)
{
    return obj.at(key).as_int64();
}

// 定义通用的 JSON 对象成员访问函数
template <typename T>
T getObjValue(const boost::json::object &obj, const std::string &key)
{
    if (obj.find(key) != obj.end())
        return get_member_impl<T>(obj, key);
    else
        return T();
}
/**
 * 获取 boost::json::object 的数组内容
 */

// 定义访问 string 类型元素的模板函数
template <typename T>
typename std::enable_if<std::is_same<T, std::string>::value, T>::type
get_array_elem_impl(const boost::json::value &elem)
{
    return std::string(elem.as_string().data(), elem.as_string().size());
}

// 定义访问 int64_t 类型元素的模板函数
template <typename T>
typename std::enable_if<std::is_same<T, int64_t>::value, T>::type
get_array_elem_impl(const boost::json::value &elem)
{
    return elem.as_int64();
}

// 定义通用的 JSON 对象数组元素访问函数
template <typename T>
std::vector<T> getArrayValue(const boost::json::object &obj, const std::string &key)
{
    std::vector<T> result;
    if (obj.find(key) != obj.end())
    {
        try
        {
            const boost::json::array &arr = obj.at(key).as_array();
            for (const auto &elem : arr)
            {
                result.push_back(get_array_elem_impl<T>(elem));
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    return result;
}

// 解析HTTP请求头
std::shared_ptr<HttpServletRequest> parse_request_header(const std::string &request_str)
{
    // 分离请求行和请求头
    auto headers_start = request_str.find("\r\n");
    if (headers_start == std::string::npos)
    {
        throw std::runtime_error("Invalid request: no headers found");
    }
    std::string request_line = request_str.substr(0, headers_start);
    auto body_start = request_str.find("\r\n\r\n");
    std::string headers = request_str.substr(headers_start + 2, body_start);

    // 解析请求行
    std::vector<std::string> tokens;
    boost::split(tokens, request_line, boost::is_any_of(" "));
    if (tokens.size() != 3)
    {
        throw std::runtime_error("Invalid request: malformed request line");
    }
    auto method = tokens[0] == "GET" ? HttpServletRequest::GET : (tokens[0] == "POST" ? HttpServletRequest::POST : HttpServletRequest::OPTIONS);
    auto uri = tokens[1];
    auto version = tokens[2];

    std::shared_ptr<HttpServletRequest> request(new HttpRequest(method, uri, version));

    // 解析请求参数
    auto query_start = uri.find('?');
    if (query_start != std::string::npos)
    {
        std::string query_string = uri.substr(query_start + 1);
        std::vector<std::string> params;
        boost::split(params, query_string, boost::is_any_of("&"));
        for (auto &param : params)
        {
            std::vector<std::string> kv;
            boost::split(kv, param, boost::is_any_of("="));
            if (kv.size() == 2)
            {
                (request)->add_parameter(kv[0], kv[1]);
            }
        }
        uri = uri.substr(0, query_start);
        request->set_path(uri);
    }

    // 解析请求头
    std::vector<std::string> header_lines;
    boost::split(header_lines, headers, boost::is_any_of("\r\n"));
    for (auto &header : header_lines)
    {
        auto delimiter_pos = header.find(":");
        if (delimiter_pos != std::string::npos)
        {
            std::string header_name = header.substr(0, delimiter_pos);
            std::string header_value = header.substr(delimiter_pos + 1);
            boost::algorithm::trim(header_value);
            if (header_name == "Authorization")
            {
                header_value = header.substr(delimiter_pos + 9);
            }
            request->add_header(header_name, header_value);
        }
    }
    if (request->get_method() == HttpServletRequest::OPTIONS)
        return request;
    // 解析请求体（仅对POST请求有效）
    if (method == HttpServletRequest::POST)
    {
        auto content_type = request->get_header("Content-Type");
        if (content_type == "application/json")
        {
            if (body_start != std::string::npos)
            {
                auto body = request_str.substr(body_start + 4);
                std::istringstream is(body);
                request->set_body(is);
            }
        }
        else if (content_type == "application/x-www-form-urlencoded")
        {
            auto body_start = request_str.find("\r\n\r\n");
            if (body_start != std::string::npos)
            {
                auto body = request_str.substr(body_start + 4);
                std::vector<std::string> params;
                boost::split(params, body, boost::is_any_of("&"));
                for (auto &param : params)
                {
                    std::vector<std::string> kv;
                    boost::split(kv, param, boost::is_any_of("="));
                    if (kv.size() == 2)
                    {
                        request->add_parameter(kv[0], kv[1]);
                    }
                }
            }
        }
        else
        {
            throw std::runtime_error("Unsupported Content-Type: " + content_type);
        }
    }
    return request;
}

// 生成uuid
std::string getUUID()
{
    boost::uuids::random_generator generator;
    boost::uuids::uuid uuid = generator();
    return boost::uuids::to_string(uuid);
}
// 生成token
std::string generate_token()
{
    // 生成16字节的随机数
    boost::random::mt19937 rng(std::time(nullptr));
    boost::random::uniform_int_distribution<> dist(0, 255);
    std::vector<unsigned char> buf(16);
    std::generate(buf.begin(), buf.end(), [&]()
                  { return dist(rng); });

    // 将随机数编码为16进制字符串
    std::string hex_str;
    boost::algorithm::hex(buf, std::back_inserter(hex_str));

    // 生成UUID并将其编码为16进制字符串
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    std::string uuid_str = boost::uuids::to_string(uuid);

    // 将随机数和UUID的16进制字符串拼接起来
    std::string token = hex_str + uuid_str;

    return token;
}
// 保存token,
void save_token(std::shared_ptr<mysqlx::abi2::r0::Session> conn, const std::string &token, std::string account)
{
    try
    {
        std::string sql = "insert into token(token, account) values(?,?)";
        conn->sql(sql).bind(token, account).execute();
    }
    catch (const mysqlx::Error &e)
    {
        HttpdLog::Error(e.what(), (char *)__func__);
    }
}
// 移除token
void delete_token(std::shared_ptr<mysqlx::abi2::r0::Session> conn, const std::string &token)
{
    try
    {
        std::string sql = " select token from token where token = ? ";
        auto res = conn->sql(sql).bind(token).execute();
        if (res.count() != 0)
        {
            sql = "delete from token where token = ?";
            conn->sql(sql).bind(token).execute();
        }
    }
    catch (const mysqlx::Error &e)
    {
        HttpdLog::Error(e.what(), (char *)__func__);
    }
}
// 查询token是否过期
bool get_token(std::shared_ptr<mysqlx::abi2::r0::Session> conn, const std::string &token)
{
    try
    {
        std::string sql = "select token from token where token = ?";
        auto res = conn->sql(sql).bind(token).execute();
        mysqlx::abi2::r0::Row row = res.fetchOne();
        if (!row.isNull())
        {
            return true;
        }
    }
    catch (const mysqlx::Error &e)
    {
        HttpdLog::Error(e.what(), (char *)__func__);
    }
    return false;
}
// 异常处理
void handle_excepiton(std::exception_ptr eptr)
{
    try
    {
        if (eptr)
        {
            std::rethrow_exception(eptr);
        }
    }
    catch (const std::exception &e)
    {
        HttpdLog::Error(e.what(), (char *)__func__);
    }
}
// 获取当前时间 yyyy-MM-dd HH:mm:ss
std::string LocalTime(std::string format)
{
    time_t now;
    struct tm *p;
    time(&now);
    p = localtime(&now);
    char time[256];
    strftime(time, sizeof(time), format.c_str(), p);
    return time;
}

_HTTPD_END_
