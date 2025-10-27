#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <memory>
#include <unordered_map>
#include <functional>

#include "../log/log.h"
#include "../pool/threadpool.h"
#include "../pool/sql_connect_pool.h"
#include "../http/http_connect.h"
#include "../heap_timer/heap_timer.h"
#include "epoller.h"

class WebServer
{
public:
    // 构造函数，初始化服务器参数
    WebServer(int port, int trigger_mode, int timeout_ms,
              int sql_port, const char *sql_user, const char *sql_pwd,
              const char *db_name, int conn_pool_num, int thread_num,
              bool open_log, int log_level, int log_que_size);
    ~WebServer();
    void start();

private:
    static int SetFdNonBlock(int fd);

    bool InitSocker();
    void InitEventMode(int trigger_mode);

    void DealListen();
    void DealRead(HttpConnect *client);
    void DealWrite(HttpConnect *client);

    void AddClient(int fd, sockaddr_in addr);
    void SendError(int fd, const char *info);
    void ExtendTime(HttpConnect *client);
    void CloseConn(HttpConnect *client);

    void OnRead(HttpConnect *client);
    void OnProcess(HttpConnect *client);
    void OnWrite(HttpConnect *client);

    static const int MAX_FD = 65536;

    int port_;
    bool open_linger_;
    int timeout_ms_;
    bool is_close_;
    int listen_fd_;
    char *src_dir_;

    uint32_t listen_event_;
    uint32_t conn_event_;

    std::unique_ptr<HeapTimer> timer_;
    std::unique_ptr<ThreadPool> thread_pool_;
    std::unique_ptr<Epoller> epoller_;
    std::unordered_map<int, HttpConnect> users_;
};

#endif // WEB_SERVER_H
