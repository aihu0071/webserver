#include "../code/http/http_response.h"
#include <iostream>
#include <cassert>


void TestHttpResponse() {
    Log* logger = Log::GetInstance();
    logger->Init(0, "./logs/", ".log", 1024);

    // 正常情况的测试
    HttpResponse response;
    std::string path = "/welcome.html";
    std::string src_dir = "/home/aihu/webserver/resources";
    int code = 200;
    bool isKeepAlive = true;

    response.Init(path, src_dir, code, isKeepAlive);

    // 测试构建响应
    Buffer buff;
    response.MakeResponse(buff);
    assert(response.Code() == 200);

    std::string str = buff.RetrieveAllAsString();
    LOG_DEBUG("http response:\n%s", str.c_str());

    // 测试错误路径
    src_dir = "/error/for/test";
    response.Init(path, src_dir, code, isKeepAlive);
    response.MakeResponse(buff);
    assert(response.Code() == 404);

    str = buff.RetrieveAllAsString();
    LOG_DEBUG("http response:\n%s", str.c_str());
}

int main() {
    TestHttpResponse();
    return 0;
}
