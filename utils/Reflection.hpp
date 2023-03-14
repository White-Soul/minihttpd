#pragma once
#include <iostream>
#include <string>
#include <tuple>

// 定义一个反射类，用于获取类的属性和方法信息
template <typename T>
struct Reflection
{
    using type = T;

    // 获取属性类型
    template <typename U, U>
    struct Member;

    template <typename C, typename M>
    struct Member<M C::*, &M::operator()>
    {
        using type = decltype(std::declval<C>().*std::declval<M>());
    };

    // 获取属性名字
    template <typename U, U>
    struct Name;

    template <typename C, typename M>
    struct Name<M C::*, &M::operator()>
    {
        static const char *value()
        {
            return __PRETTY_FUNCTION__;
        }
    };

    // 获取方法参数类型和返回值类型
    template <typename U, U>
    struct Method;

    template <typename C, typename R, typename... Args>
    struct Method<R (C::*)(Args...), &C::getInfo>
    {
        using result_type = R;
        using arg_types = std::tuple<Args...>;
    };

    // 获取属性信息
    template <typename M>
    static auto get_member_info(M m) -> std::tuple<const char *, typename Member<M, &T::name>::type *>
    {
        return std::make_tuple(Name<M, &T::name>::value(), &(std::declval<T>().*m));
    }

    // 获取方法信息
    template <typename M>
    static auto get_method_info(M m) -> std::tuple<const char *, typename Method<M, &T::getInfo>::result_type, typename Method<M, &T::getInfo>::arg_types>
    {
        return std::make_tuple(Name<M, &T::getInfo>::value(), (std::declval<T>().*m)(), std::tuple<typename std::decay<Args>::type...>());
    }
};