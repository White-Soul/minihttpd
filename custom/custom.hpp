#pragma once
#include <string>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include "../utils/utils.hpp"
using namespace httpd;
class User : public Custom<User>
{
public:
    std::string name;
    std::string account;
    std::string password;
    std::string email;
    std::string phone;
    std::string address;
    std::string qq;
    std::string wchat;
    int isRoot;

    std::string getId() const{
        return account;
    }

    std::string serialize() const override
    {
        std::ostringstream os;
        boost::property_tree::ptree pt;
        pt.put("name", name);
        pt.put("account", account);
        pt.put("password", password);
        pt.put("isRoot", isRoot);
        pt.put("email", email);
        pt.put("phone", phone);
        pt.put("address", address);
        pt.put("qq", qq);
        pt.put("wchat", wchat);
        boost::property_tree::write_json(os, pt, false);
        return os.str();
    }

    static User deserialize(const std::string &json)
    {
        User u;
        boost::property_tree::ptree tree;
        std::istringstream ss(json);
        boost::property_tree::read_json(ss, tree);
        u.name = getPtreeNode<std::string>(tree, "name");
        u.account = getPtreeNode<std::string>(tree, "account");
        u.password = getPtreeNode<std::string>(tree, "password");
        u.email = getPtreeNode<std::string>(tree, "email");
        u.phone = getPtreeNode<std::string>(tree, "phone");
        u.isRoot = getPtreeNode<int>(tree, "isRoot");
        u.address = getPtreeNode<std::string>(tree, "address");
        u.qq = getPtreeNode<std::string>(tree, "qq");
        u.wchat = getPtreeNode<std::string>(tree, "wchat");
        return u;
    }
};

class Asset : public Custom<Asset>
{
public:
    int money;
    int amount;
    std::string name;
    std::string desc;
    std::string code;
    std::string type_id;
    std::string user_account;

    std::string serialize() const
    {
        std::ostringstream os;
        boost::property_tree::ptree pt;
        pt.put("code", code);
        pt.put("name", name);
        pt.put("desc", desc);
        pt.put("amount", amount);
        pt.put("money", money);
        pt.put("type_id", type_id);
        pt.put("user_account", user_account);
        boost::property_tree::write_json(os, pt, false);
        return os.str();
    }

    std::string getId() const{
        return code;
    }

    static Asset deserialize(const std::string &json)
    {
        Asset u;
        boost::property_tree::ptree tree;
        std::istringstream ss(json);
        boost::property_tree::read_json(ss, tree);
        u.code = getPtreeNode<std::string>(tree, "code");
        u.name = getPtreeNode<std::string>(tree, "name");
        u.desc = getPtreeNode<std::string>(tree, "desc");
        u.amount = getPtreeNode<int>(tree, "amount");
        u.money = getPtreeNode<int>(tree, "money");
        u.type_id = getPtreeNode<std::string>(tree, "type_id");
        u.user_account = getPtreeNode<std::string>(tree, "user_account");
        return u;
    }
};

class Type : public Custom<Type>
{
public:
    std::string id;
    std::string type_name;
    std::string serialize() const
    {
        std::ostringstream os;
        boost::property_tree::ptree pt;
        pt.put("id", id);
        pt.put("type_name", type_name);
        boost::property_tree::write_json(os, pt, false);
        return os.str();
    }

    std::string getId() const {
        return id;
    }

    static Type deserialize(const std::string &json)
    {
        Type u;
        boost::property_tree::ptree tree;
        std::istringstream ss(json);
        boost::property_tree::read_json(ss, tree);
        u.id = getPtreeNode<int>(tree, "id");
        u.type_name = getPtreeNode<std::string>(tree, "id");

        return u;
    }
};

class Log : public Custom<Log>
{
public:
    std::string message;
    std::string date;

    std::string serialize() const
    {
        std::ostringstream os;
        boost::property_tree::ptree pt;
        pt.put("message", message);
        pt.put("date", date);
        boost::property_tree::write_json(os, pt, false);
        return os.str();
    }

    std::string getId() const {
        return "";
    }

    static Log deserialize(const std::string &json)
    {
        Log u;
        boost::property_tree::ptree tree;
        std::istringstream ss(json);
        boost::property_tree::read_json(ss, tree);
        u.message = getPtreeNode<std::string>(tree, "message");
        u.date = getPtreeNode<std::string>(tree, "date");

        return u;
    }
};
