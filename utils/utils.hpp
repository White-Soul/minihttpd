#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <mysqlx/xdevapi.h>
#include <boost/json.hpp>
#include <boost/json/src.hpp>
#include "Result.hpp"
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
    boost::property_tree::ptree pt;
    std::istringstream is(body);
    read_json(is, pt);
    boost::json::value json_val = boost::json::parse(body);
    const boost::json::object &data_obj = json_val.at("data").as_object();
    return data_obj;
}
/**
 * 获取 boost::json::object 的内容
 * 若失败则返回T的默认构造对象
 */
template <class T>
T getObjValue(const boost::json::object &obj, const std::string &str)
{
    try
    {
        const auto &value = obj.at(str);
        return value.as<T>();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return T();
    }
}
/**
 * 获取 boost::json::object 的数组内容
*/
template <class T>
std::vector<T> getArrayValue(const boost::json::object &obj, const std::string &str) {
    const boost::json::array &arr = data_obj.at(str).as_array();
    std::vector<T> result;
    for (const auto &elem : arr) {
        try {
            result.push_back(elem.as<T>());
        } catch (const std::exception &e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    return result;
}

