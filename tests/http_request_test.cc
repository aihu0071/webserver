#include "../code/http/http_request.h"
#include <iostream>

void TestHttpRequest() {
    // 初始化日志系统
    Log* logger = Log::GetInstance();
    logger->Init(0, "./logs/", ".log", 1024);

    // 初始化测试
    HttpRequest request;

    // 初始化数据库连接池
    SqlConnectPool* conn_pool = SqlConnectPool::instance();
    conn_pool->Init("localhost", 3306, "aihu", "password", "web_server", 10);


    // 解析请求测试
    std::string http_request = "GET /index.html HTTP/1.1\r\n"
                               "Host: example.com\r\n"
                               "Connection: keep-alive\r\n"
                               "\r\n";
    Buffer buff;
    buff.Append(http_request);
    bool parseResult = request.Parse(buff);
    assert(parseResult);

    // 访问器方法测试
    assert(request.Method() == "GET");
    assert(request.Path() == "/index.html");
    assert(request.Version() == "1.1");

    // 模拟注册 POST 请求
    std::string register_request = "POST /register.html HTTP/1.1\r\n"
                                   "Host: example.com\r\n"
                                   "Content-Type: application/x-www-form-urlencoded\r\n"
                                   "Content-Length: 29\r\n"
                                   "\r\n"
                                   "username=test&password=123456";
    request.Init();
    buff.Append(register_request);
    parseResult = request.Parse(buff);
    assert(parseResult);

    assert(request.Method() == "POST");
    assert(request.Path() == "/welcome.html");
    assert(request.Version() == "1.1");
    assert(request.GetPost("username") == "test");
    assert(request.GetPost("password") == "123456");

    // Post 数据获取测试
    // 模拟登录 POST 请求
    std::string login_request = "POST /login HTTP/1.1\r\n"
                               "Content-Type: application/x-www-form-urlencoded\r\n"
                               "Content-Length: 29\r\n"
                               "\r\n"
                               "username=test&password=123456"
                               "\r\n";
    buff.Append(login_request);
    request.Init();

    parseResult = request.Parse(buff);
    assert(parseResult);

    assert(request.Method() == "POST");
    assert(request.Path() == "/welcome.html");
    assert(request.Version() == "1.1");
    assert(request.GetPost("username") == "test");
    assert(request.GetPost("password") == "123456");

    std::cout << "All tests passed!" << std::endl;
}

int main() {
    TestHttpRequest();
    return 0;
}
