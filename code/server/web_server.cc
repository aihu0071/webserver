#include "web_server.h"

WebServer::WebServer(int port, int trigger_mode, int timeout_ms,
                     int sql_port, const char* sql_user, const char* sql_pwd,
                     const char* db_name, int conn_pool_num, int thread_num, 
                     bool open_log, int log_level, int log_que_size) 
                     : port_(port), timeout_ms_(timeout_ms), is_close_(false), 
                       timer_(new HeapTimer()), thread_pool_(new ThreadPool(thread_num)), 
                       epoller_(new Epoller()) {
    // 初始化日志
    if(open_log) {
        Log::GetInstance()->Init(log_level, "./logs/", ".log", log_que_size);
        fprintf(stderr, "Log initialized, IsOpen=%d, level=%d\n", Log::GetInstance()->IsOpen(), Log::GetInstance()->GetLevel());
        LOG_DEBUG("测试日志系统是否工作"); 
        Log::GetInstance()->Write(0, "直接测试写入");
        if (is_close_) { 
            LOG_ERROR("============== Server Init Error! =============="); 
        } else {
             
            LOG_INFO("============== Server Init! ==============");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                    (listen_event_ & EPOLLET) ? "ET" : "LT",
                    (conn_event_ & EPOLLET) ? "ET" : "LT");
            LOG_INFO("LogSys level: %d", log_level);
            LOG_INFO("src_dir: %s", HttpConnect::src_dir);
            LOG_INFO("SqlConnectPool num: %d, ThreadPool num: %d", conn_pool_num, thread_num);
            fprintf(stderr, "Log initialized, IsOpen=%d, level=%d\n", Log::GetInstance()->IsOpen(), Log::GetInstance()->GetLevel());
        }
    }
    HttpConnect::use_count = 0;
    src_dir_ = getcwd(nullptr, 256); // 获取当前工作目录
    assert(src_dir_);
    strcat(src_dir_, "/resources/");
    HttpConnect::src_dir = src_dir_;

    SqlConnectPool::instance()->Init("localhost", sql_port, sql_user, sql_pwd, db_name, conn_pool_num);
    InitEventMode(trigger_mode);
    if(!InitSocker()){
        is_close_ = true;
    }
}

WebServer::~WebServer(){
    close(listen_fd_);
    is_close_ = true;
    free(src_dir_);
    SqlConnectPool::instance()->ClosePool();
}

void WebServer::start(){
    int time_ms = -1; // epoll_wait 超时时间，-1表示无限等待
    if(!is_close_){
        LOG_INFO("============== Server Start ==============");}
    while(!is_close_){
        if (timeout_ms_ > 0) {
            time_ms = timer_->GetNextTick();  
        }  
        int event_count = epoller_->Wait(time_ms);
        for(int i = 0; i < event_count; ++i){
            int fd = epoller_->GetEventsFd(i);
            uint32_t events = epoller_->GetEvents(i);
            if(fd == listen_fd_){ // 处理新连接
                DealListen();
            }
            else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){ // 处理异常事件
                assert(users_.count(fd) > 0);
                CloseConn(&users_[fd]);
            }
            else if(events & EPOLLIN){ // 处理读事件
                assert(users_.count(fd) > 0);
                DealRead(&users_[fd]);
            }
            else if(events & EPOLLOUT){ // 处理写事件
                assert(users_.count(fd) > 0);
                DealWrite(&users_[fd]);
            }
        }
    }
}



int WebServer::SetFdNonBlock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}



bool WebServer::InitSocker(){
    int ret;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);

    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd_ < 0){
        LOG_ERROR("Create socket error!", port_);
        return false;
    }

    int optval = 1;
    ret = setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if(ret  == -1){
        LOG_ERROR("Set socket setsockopt error!", port_);
        return false;
    }

    ret = bind(listen_fd_, (struct sockaddr*)&addr, sizeof(addr));
    if(ret < 0){
        LOG_ERROR("Bind port: %d error!", port_);
        return false;
    }

    ret = listen(listen_fd_, 8);
    if(ret < 0){
        LOG_ERROR("Listen port: %d error!", port_);
        close(listen_fd_);
        return false;
    }

    ret = epoller_->AddFd(listen_fd_, listen_event_ | EPOLLIN);
    if(ret == 0){
        LOG_ERROR("Add listen fd to epoll error!", port_);
        close(listen_fd_);
        return false;
    }
    
    SetFdNonBlock(listen_fd_);
    LOG_INFO("Server port: %d init success!", port_);
    return true;
}

void WebServer::InitEventMode(int trigger_mode) {
    listen_event_ = EPOLLRDHUP; // 检测socket关闭
    conn_event_ = EPOLLONESHOT | EPOLLRDHUP; // EPOLLONESHOT由一个线程处理
    switch (trigger_mode) {
    case 0:
        break;
    case 1:
        conn_event_ |= EPOLLET;
        break;
    case 2:
        listen_event_ |= EPOLLET;
        break;
    case 3:
        listen_event_ |= EPOLLET;
        conn_event_ |= EPOLLET;
        break;
    default: 
        listen_event_ |= EPOLLET;
        conn_event_ |= EPOLLET;
        break;
    }
    HttpConnect::is_ET = (conn_event_ & EPOLLET);
}

void WebServer::AddClient(int fd, sockaddr_in addr) {
    assert(fd > 0);
    users_[fd].Init(fd, addr);
    if (timeout_ms_ > 0) {
        timer_->Add(fd, timeout_ms_, std::bind(&WebServer::CloseConn, this, &users_[fd]));
    }
    epoller_->AddFd(fd, EPOLLIN | conn_event_);
    SetFdNonBlock(fd);
    LOG_INFO("Client[%d] in!", users_[fd].GetFd());
}

void WebServer::DealListen() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do {
        int fd = accept(listen_fd_, (sockaddr*)&addr, &len);
        if (fd <= 0) { return; }
        else if (HttpConnect::use_count >= MAX_FD) {
            SendError(fd, "Server busy!");
            LOG_WARN("Clients is full!");
            return;
        }
        AddClient(fd, addr);
    } while (listen_event_ & EPOLLET);
}

void WebServer::DealRead(HttpConnect* client) {
    assert(client);
    ExtendTime(client);
    thread_pool_->AddTask(std::bind(&WebServer::OnRead, this, client));
}

void WebServer::DealWrite(HttpConnect* client) {
    assert(client);
    ExtendTime(client);
    thread_pool_->AddTask(std::bind(&WebServer::OnWrite, this, client));
}

void WebServer::OnRead(HttpConnect* client) {
    assert(client);
    int ret = -1;
    int read_errno = 0;
    ret = client->Read(&read_errno);
    if (ret <= 0 && read_errno != EAGAIN) {
        CloseConn(client);
        return;
    }
    OnProcess(client);
}

void WebServer::OnProcess(HttpConnect* client) {
    if (client->Process()) {
        epoller_->ModFd(client->GetFd(), conn_event_ | EPOLLOUT); // 监听写
    } else {
        epoller_->ModFd(client->GetFd(), conn_event_ | EPOLLIN); // 监听读
    }
}

void WebServer::OnWrite(HttpConnect* client) {
    assert(client);
    int ret = 0;
    int write_errno = 0;
    ret = client->Write(&write_errno);
    if (client->ToWriteBytes() == 0) {
        if (client->IsKeepAlive()) {
            epoller_->ModFd(client->GetFd(), conn_event_ | EPOLLIN); // 监听读
        }
    } else if (ret < 0) {
        if (write_errno == EAGAIN) {
            epoller_->ModFd(client->GetFd(), conn_event_ | EPOLLOUT); // 监听写
            return;
        }
    }
    CloseConn(client);
}

void WebServer::CloseConn(HttpConnect* client) {
    assert(client);
    LOG_INFO("Client[%d] quit!", client->GetFd());
    epoller_->DelFd(client->GetFd());
    client->Close();
}

void WebServer::SendError(int fd, const char* info) {
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if (ret < 0) {
        LOG_WARN("Send error to client[%d] error!", fd);
    }
    close(fd);
}

void WebServer::ExtendTime(HttpConnect* client) {
    assert(client);
    if (timeout_ms_ > 0) {
        timer_->Adjust(client->GetFd(), timeout_ms_);
    }
}