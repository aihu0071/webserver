cmake_minimum_required(VERSION 3.10)
project(webserver)

# 设置 C++ 标准和编译器选项
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# 设置可执行文件输出路径为 build 目录的上一层
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/..)

set(COMMON ./buffer/buffer.cc ./log/log.cc)
set(SQL_POOL ./pool/sql_connect_pool.cc)
set(HTTP  ./http/http_request.cc ./http/http_response.cc ./http/http_connect.cc)
set(HEAP_TIMER ./heap_timer/heap_timer.cc)
set(SERVER ./server/epoller.cc ./server/web_server.cc)

# 查找 MySQL 库
find_package(PkgConfig REQUIRED)
pkg_check_modules(MYSQL REQUIRED mysqlclient)
# 包含 MySQL 头文件目录
include_directories(${MYSQL_INCLUDE_DIR})

add_executable(webserver main.cc ${COMMON} ${SQL_POOL} ${HTTP} ${HEAP_TIMER} ${SERVER})
target_link_libraries(webserver ${MYSQL_LIBRARIES} pthread)