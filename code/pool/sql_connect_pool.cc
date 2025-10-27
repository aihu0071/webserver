#include "sql_connect_pool.h"

SqlConnectPool* SqlConnectPool::instance(){
    static SqlConnectPool conn_pool;
    return &conn_pool;
}

void SqlConnectPool::Init(const char* host, int port,
                           const char* user, const char* pwd,
                           const char* db_name, int connect_size){
    assert(connect_size > 0);
    for(int i = 0; i < connect_size; i++){
        MYSQL* connect = nullptr;
        connect = mysql_init(nullptr);
        if(!connect){
            LOG_ERROR("MySQL init fail!");
            assert(connect);
        }
        if(!mysql_real_connect(connect, host, user, pwd, db_name, port, nullptr, 0)){
            LOG_ERROR("MySql failed to connect to database: Error: %s", mysql_error(connect));
        } else {
            connetc_queue_.emplace(connect);
            max_conn_++;
        }
    }
    assert(max_conn_ > 0);
    //设置线程级信号量,初始值为最大连接数,用于控制连接的获取和释放
    sem_init(&sem_id, 0, max_conn_);
}

void SqlConnectPool::ClosePool(){
    std::lock_guard<std::mutex>locker(mtx_);
    while(!connetc_queue_.empty()){
        MYSQL* conn = connetc_queue_.front();
        connetc_queue_.pop();
        mysql_close(conn);
    }
}




MYSQL* SqlConnectPool::GetConnect(){
    MYSQL* conn = nullptr;
    if(connetc_queue_.empty()){
        LOG_WARN("SqlConnectPool busy!");
        return nullptr;
    }
    //等待信号量，获取一个连接，信号量减1
    sem_wait(&sem_id);
    std::lock_guard<std::mutex>locker(mtx_);
    conn = connetc_queue_.front();
    connetc_queue_.pop();
    return conn;
}

void SqlConnectPool::FreeConnect(MYSQL* conn){
    assert(conn);
    std::lock_guard<std::mutex>locker(mtx_);
    connetc_queue_.push(conn);
    //释放一个连接，信号量加1
    sem_post(&sem_id);
}

int SqlConnectPool::GetFreeConnCount(){
    std::lock_guard<std::mutex>locker(mtx_);
    return connetc_queue_.size();
}