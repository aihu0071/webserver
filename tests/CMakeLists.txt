# cmake_minimum_required(VERSION 3.10)
# project(buffer_unit_test)

# # 设置 C++ 标准和编译器选项
# # set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_CXX_STANDARD 14)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# # 定义公共源文件和特定文件
# set(COMMON_SOURCES ../code/buffer/buffer.cc ../code/log/log.cc)
# set(POOL_SOURCE ../code/pool/sql_connect_pool.cc)

# # 查找 MySQL 库
# find_package(PkgConfig REQUIRED)
# find_package(Threads REQUIRED)
# pkg_check_modules(MYSQL REQUIRED mysqlclient)
# # 包含 MySQL 头文件目录
# include_directories(${MYSQL_INCLUDE_DIR})

# # 添加可执行文件
# add_executable(sql_connect_pool_test sql_connect_pool_test.cc ${COMMON_SOURCES} ${POOL_SOURCE})

# # 链接库
# target_link_libraries(sql_connect_pool_test 
#     ${MYSQL_LIBRARIES} 
#     ${CMAKE_THREAD_LIBS_INIT} 
#     pthread)








# cmake_minimum_required(VERSION 3.10)
# project(tests)

# # 设置 C++ 标准和编译器选项
# set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_CXX_STANDARD 14)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# # 定义公共源文件和特定文件
# set(COMMON_SOURCES ../code/buffer/buffer.cc ../code/log/log.cc)
# set(HTTP_SOURCE ../code/http/http_request.cc)
# set(POOL_SOURCE ../code/pool/sql_connect_pool.cc)

# # 查找 MySQL 库
# find_package(PkgConfig REQUIRED)
# find_package(Threads REQUIRED)
# pkg_check_modules(MYSQL REQUIRED mysqlclient)
# # 包含 MySQL 头文件目录
# include_directories(${MYSQL_INCLUDE_DIR})

# # 添加可执行文件
# add_executable(http_request_test http_request_test.cc ${COMMON_SOURCES} ${HTTP_SOURCE} ${POOL_SOURCE})
# # 链接库
# target_link_libraries(http_request_test 
#     ${MYSQL_LIBRARIES} 
#     ${CMAKE_THREAD_LIBS_INIT} 
#     pthread)





# cmake_minimum_required(VERSION 3.10)
# project(tests)

# # 设置 C++ 标准和编译器选项
# set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_CXX_STANDARD 14)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")


# find_package(Threads REQUIRED)

# # 定义公共源文件和特定文件
# set(COMMON ../code/buffer/buffer.cc ../code/log/log.cc)
# set(HTTP_REQUEST ../code/http/http_request.cc)
# set(HTTP_RESPONSE ../code/http/http_response.cc)
# set(SQL_POOL ../code/pool/sql_connect_pool.cc)

# add_executable(http_response_test http_response_test.cc ${COMMON} ${HTTP_RESPONSE})

# target_link_libraries(http_response_test 
#     ${CMAKE_THREAD_LIBS_INIT} 
#     pthread)



# cmake_minimum_required(VERSION 3.10)
# project(tests)

# # 设置 C++ 标准和编译器选项
# set(CMAKE_CXX_STANDARD 14)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# # 定义公共源文件和特定文件
# set(COMMON ../code/buffer/buffer.cc ../code/log/log.cc)
# set(SQL_POOL ../code/pool/sql_connect_pool.cc)
# set(HTTP_REQUEST ../code/http/http_request.cc)
# set(HTTP_RESPONSE ../code/http/http_response.cc)
# set(HTTP_CONNECT ../code/http/http_connect.cc)
# set(HTTP ../code/pool/sql_connect_pool.cc ../code/http/http_request.cc 
#          ../code/http/http_response.cc ../code/http/http_connect.cc)

# # 查找 MySQL 库
# find_package(PkgConfig REQUIRED)
# find_package(Threads REQUIRED)
# pkg_check_modules(MYSQL REQUIRED mysqlclient)
# # 包含 MySQL 头文件目录
# include_directories(${MYSQL_INCLUDE_DIR})

# add_executable(http_connect_test http_connect_test.cc ${COMMON} ${HTTP})
# target_link_libraries(http_connect_test 
#     ${MYSQL_LIBRARIES}
#     ${CMAKE_THREAD_LIBS_INIT} 
#     pthread)






cmake_minimum_required(VERSION 3.10)
project(tests)

# 设置 C++ 标准和编译器选项
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")



find_package(Threads REQUIRED)

set(COMMON ../code/buffer/buffer.cc ../code/log/log.cc)
set(HEAP_TIMER ../code/heap_timer/heap_timer.cc)

add_executable(heap_timer_test heap_timer_test.cc ${COMMON} ${HEAP_TIMER})


target_link_libraries(heap_timer_test 
    ${CMAKE_THREAD_LIBS_INIT} 
    pthread)