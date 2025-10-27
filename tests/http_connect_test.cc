#include "../code/http/http_connect.h"
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

// 创建一对本地套接字
void CreateSocketPair(int& sock1, int& sock2) {
    int socks[2];
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, socks) == 0);
    sock1 = socks[0];
    sock2 = socks[1];
}

void TestHttpConnect() {
    Log* logger = Log::GetInstance();
    logger->Init(0, "./logs/", ".log", 1024);

    int client_sock, server_sock;
    CreateSocketPair(client_sock, server_sock);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);

    HttpConnect conn;
    conn.Init(server_sock, addr);

    // 测试Read函数
    {
        int save_errno;
        std::string http_request = "GET /index.html HTTP/1.1\r\n"
                                   "Host: example.com\r\n"
                                   "Connection: keep-alive\r\n"
                                   "\r\n";
        // client_sock 和 server_sock 是相互连接的
        // 以写入到 client_sock 的数据会被自动传输到 server_sock 的接收缓冲区中。
        write(client_sock, http_request.c_str(), http_request.size());
        ssize_t len = conn.Read(&save_errno);
        assert(len == (ssize_t)http_request.size());
    }

    // 测试Process函数
    {
        HttpConnect::src_dir = "/home/aihu/webserver/resources";
        bool result = conn.Process();
        assert(result);
    }

    // 测试Write函数
    {
        int saveErrno;
        ssize_t len = conn.Write(&saveErrno);
        assert(len >= -1);

        char buffer[10240];
        ssize_t recv_len = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
        assert(recv_len == len);
    }
    close(client_sock);
    close(server_sock);
}

int main() {
    TestHttpConnect();
    return 0;
}
