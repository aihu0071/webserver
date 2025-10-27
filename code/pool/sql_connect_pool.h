#ifndef SQL_CONNECT_POOL_H
#define SQL_CONNECT_POOL_H

#include <string>
#include <queue>
#include <mutex>
#include <thread>
#include <cassert>
#include <semaphore.h>
#include <mysql/mysql.h>

#include "../log/log.h"

class SqlConnectPool {
public:
    static SqlConnectPool* instance();

    void Init(const char* host, int port,
              const char* user, const char* pwd,
              const char* db_name, int connect_size = 10);
    void ClosePool();
    MYSQL* GetConnect();

    void FreeConnect(MYSQL* conn);
    int GetFreeConnCount();

private:
    SqlConnectPool() = default;
    ~SqlConnectPool(){
        ClosePool();
    };


    int max_conn_; //最大连接数
    std::queue<MYSQL*> connetc_queue_; //连接队列
    std::mutex mtx_;
    sem_t sem_id;
};

class SqlConnectRAII {
public:
    SqlConnectRAII(MYSQL** sql, SqlConnectPool* pool) {
        assert(pool);
        *sql = pool->GetConnect();
        sql_ = *sql;
        pool_ = pool;
    }

    ~SqlConnectRAII(){
        if(sql_){
            pool_->FreeConnect(sql_);
        }
    }
private:
    MYSQL* sql_;
    SqlConnectPool* pool_;
};

#endif