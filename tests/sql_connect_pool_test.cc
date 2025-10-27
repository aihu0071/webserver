#include "../code/pool/sql_connect_pool.h"
#include <iostream>

// 测试 SqlConnectPool 类的功能
void TestSqlConnectPool() {
    // 初始化日志系统
    Log* logger = Log::GetInstance();
    logger->Init(0, "./logs/", ".log", 1024);

    // 获取 SqlConnectPool 的单例实例
    SqlConnectPool* pool = SqlConnectPool::instance();

    // 初始化连接池
    const char* host = "localhost";
    uint16_t port = 3306;
    const char* user = "aihu";
    const char* pwd = "password";
    const char* db_name = "web_server";
    int connect_size = 10;
    pool->Init(host, port, user, pwd, db_name, connect_size);

    // 测试获取连接
    MYSQL* conn = pool->GetConnect();
    assert(conn != nullptr);  // 确保获取到的连接不为空

    // 执行简单的 SQL 查询
    if (mysql_query(conn, "select 1")) {
        std::cerr << "query failed: " << mysql_error(conn) << std::endl;
    } else {
        MYSQL_RES* result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row) {
                std::cout << "Query result: " << row[0] << std::endl;
            }
            mysql_free_result(result);
        }
    }

    // 测试释放连接
    pool->FreeConnect(conn);

    // 测试获取空闲连接数量
    int freeCount = pool->GetFreeConnCount();
    std::cout << "Free connection count: " << freeCount << std::endl;

    // 测试 SqlConnRAII 类
    {
        MYSQL* sql = nullptr;
        SqlConnectRAII raii(&sql, pool);
        assert(sql != nullptr);  // 确保通过 RAII 获取到的连接不为空
    }  // 离开作用域，自动释放连接

    // 关闭连接池
    pool->ClosePool();
}

int main() {
    TestSqlConnectPool();
    return 0;
}