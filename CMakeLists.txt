
cmake_minimum_required(VERSION 3.0)
project(httpd)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
# 设置 _ROOT变量被忽略了，如果不想忽略需要使用cmake_policy指令设置变量CMP0074
cmake_policy(SET CMP0074 NEW) 

# 添加源文件
set(SOURCES
    httpd.cpp
)

# 设置 Boost 库的根路径，这里默认是 /usr/local
set(Boost_INCLUDE_DIR /usr/local/include)
set(Boost_LIBRARY_DIR /usr/local/lib)
set(MYSQL_CONN_CPP_INCLUDE_DIR /usr/include/mysql-cppconn-8)
set(MYSQL_CONN_CPP_LIBRARY_DIR /usr/lib64)

# 添加 Boost 库的版本号到路径中
find_package(Boost COMPONENTS system REQUIRED)

MESSAGE( STATUS "Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}")
MESSAGE( STATUS "Boost_LIBRARY_DIRS = ${Boost_LIBRARY_DIRS}")
MESSAGE( STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}")
MESSAGE( STATUS "Boost_FILESYSTEM_LIBRARY = ${Boost_FILESYSTEM_LIBRARY}")
MESSAGE( STATUS "CMAKE_BINARY_DIR = ${CMAKE_BINARY_DIR}")

# 添加utils
add_subdirectory(utils)

# 将 Boost 库的 include 路径添加到编译器的头文件搜索路径
include_directories(${Boost_INCLUDE_DIRS} ${MYSQL_CONN_CPP_INCLUDE_DIR})

# 将 Boost 库的库文件路径添加到链接器的库文件搜索路径
link_directories(${Boost_LIBRARY_DIRS} ${MYSQL_CONN_CPP_LIBRARY_DIR})

# 编译可执行文件
add_executable(httpds ${SOURCES})

# 链接 Boost 库
target_link_libraries(httpds ${Boost_LIBRARIES} -pthread -lmysqlcppconn8 utils)