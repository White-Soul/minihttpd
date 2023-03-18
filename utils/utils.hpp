#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <mysqlx/xdevapi.h>
#include <boost/json.hpp>
#include <boost/json/src.hpp>
#include <type_traits>
#include "Result.hpp"
#include "Interface.hpp"
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
const boost::json::object &body_to_obj(const std::string &body)
{
    const boost::json::object &data_obj = boost::json::parse(body).as_object();
    return data_obj;
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
