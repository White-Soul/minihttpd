cmake_minimum_required(VERSION 3.0)
# cmake .. -G "Unix Makefiles"
# 不知道为什么生成的是vs的解决方案
project(httpd)
cmake_policy(SET CMP0074 NEW)
# set(Boost_DEBUG ON)
set(CMAKE_CXX_STANDARD 11)
set(SOURCES test.cpp)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
# set(CMAKE_PREFIX_PATH "C:/Program Files (x86)/MySQL/MySQL Connector C++ 8.0/lib/vs14")
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message("32-bit compiler detected.")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    message("64-bit compiler detected.")
endif()

# set(BOOST_ROOT "C:/Boost")
# set(BOOST_INCLUDEDIR "C:/Boost/include/boost-1_79")
# set(BOOST_LIBRARYDIR "C:/Boost/lib")
# set(Boost_INCLUDE_DIRS "C:/Boost/include/boost-1_79")
# set(Boost_LIBRARY_DIRS "C:/Boost/lib")

set(MYSQL_CONNECTOR_CPP_INCLUDE_DIRS "C:/Program Files/MySQL/MySQL Connector C++ 8.0/include")
# set(MYSQL_INCLUDE_DIRS "C:/Program Files/MySQL/MySQL Server 8.0/include")
set(MYSQL_CONNECTOR_CPP_LIB_DIRS "C:/Program Files/MySQL/MySQL Connector C++ 8.0/lib64/vs14")
# set(MYSQL_LIB_DIRS "C:/Program Files/MySQL/MySQL Server 8.0/lib")
set(MYSQL_CONNECTOR_CPP_LIBRARIES mysqlcppconn)
# set(MYSQL_LIBRARIES libmysql)
# 找到boost路径
find_package(Boost REQUIRED)
# 找到mysqlcnn路径
# find_package(mysqlcppconn REQUIRED)

MESSAGE( STATUS "Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}")
MESSAGE( STATUS "Boost_LIBRARY_DIRS = ${Boost_LIBRARY_DIRS}")
MESSAGE( STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}")
MESSAGE( STATUS "Boost_FILESYSTEM_LIBRARY = ${Boost_FILESYSTEM_LIBRARY}")
MESSAGE( STATUS "CMAKE_BINARY_DIR = ${CMAKE_BINARY_DIR}")
#  ${MYSQL_CONNECTOR_CPP_INCLUDE_DIRS} ${MYSQL_CONNECTOR_CPP_INCLUDE_DIRS} ${MYSQL_INCLUDE_DIRS}
include_directories(${Boost_INCLUDE_DIRS} ${MYSQL_CONNECTOR_CPP_INCLUDE_DIRS})
# ${MYSQL_CONNECTOR_CPP_LIB_DIRS} ${MYSQL_LIB_DIRS}
link_directories(${Boost_LIBRARY_DIRS} ${MYSQL_CONNECTOR_CPP_LIB_DIRS})

add_executable(httpd ${SOURCES})
# ${MYSQL_CONNECTOR_CPP_LIBRARIES} ${MYSQL_LIBRARIES}
target_link_libraries(httpd ${Boost_LIBRARIES} ${MYSQL_CONNECTOR_CPP_LIBRARIES})