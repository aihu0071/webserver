#ifndef HTTP_CONNECT_H
#define HTTP_CONNECT_H

#include <arpa/inet.h> // sockaddr_in
#include <sys/uio.h>   // readv/writev
#include <cassert>
#include <atomic>

#include "../buffer/buffer.h"
#include "../log/log.h"
#include "http_request.h"
#include "http_response.h"

class HttpConnect {
public:
    static bool is_ET;
    static const char* src_dir;
    static std::atomic<int> use_count;

    HttpConnect();
    ~HttpConnect();

    void Init(int socket_fd, const sockaddr_in& addr);
    void Close();

    ssize_t Read(int* save_errno);
    ssize_t Write(int* save_errno);
    bool Process();

    // 写的总长度
    int ToWriteBytes() const { return iov_[0].iov_len + iov_[1].iov_len; }
    bool IsKeepAlive() const { return request_.IsKeepAlive(); }

    int GetFd() const { return fd_; }
    int GetPort() const { return addr_.sin_port; }
    const char* GetIP() const { return inet_ntoa(addr_.sin_addr); }
    sockaddr_in GetAddr() const { return addr_; }

private:
    int fd_;     //客户端连接的套接字文件描述符，用于进行网络数据的读写操作
    bool is_close_;
    struct sockaddr_in addr_;   //客户端的地址信息，包括 IP 地址和端口号

    int iov_cnt_;    //iovec 数组的有效元素数量，用于 writev 函数进行分散写操作。
    struct iovec iov_[2];   //iovec 结构体数组，用于存储待写入的数据块信息。

    Buffer read_buff_;
    Buffer write_buff_;    
    
    HttpRequest request_;
    HttpResponse response_;
};

#endif // HTTP_CONNECT_H
