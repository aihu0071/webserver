#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <cstring>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <regex> // 正则表达式
#include <mysql/mysql.h>



#include "../buffer/buffer.h"
#include "../log/log.h"
#include "../pool/sql_connect_pool.h"

class HttpRequest{
public:
    enum PARSE_STATE {
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH
    };
    HttpRequest() { Init(); }
    ~HttpRequest() = default;

    void Init(); // 初始化
    bool Parse(Buffer& buff); // 解析 HTTP 请求

    std::string Method() const;    // HTTP 请求方法
    std::string Path() const;   // 请求路径
    std::string& Path();    // 获取可修改的请求路径
    std::string Version() const;    // HTTP 版本
    std::string GetPost(const std::string& key) const;   // 获取 POST 请求数据
    std::string GetPost(const char* key) const;   // 获取 POST 请求数据

    bool IsKeepAlive() const;    // 是否保持连接

private:
    static int  ConverHex(char ch); // 十六进制转换为十进制
    static bool UserVerify(const std::string& name, const std::string& pwd, bool is_login); // 用户验证

    bool ParseRequestLine(const std::string& line); // 解析请求行
    void ParseHeader(const std::string& line); // 解析请求头
    void ParseBody(const std::string& line); // 解析请求体

    void ParsePath(); // 解析请求路径 
    void ParsePost(); // 解析 POST 请求数据
    void ParseFromUrlEncoded(); // 解析 URL 编码格式的 POST 数据 

    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;

    PARSE_STATE state_;  // 当前解析状态,初始为 REQUEST_LINE,HEADERS,BODY,FINISH
    std::string method_;  // HTTP 请求方法
    std::string path_;    // 请求路径
    std::string version_;  // HTTP 版本
    std::string body_;   // 请求体
    std::unordered_map<std::string, std::string> header_; // HTTP 请求的头部信息
    std::unordered_map<std::string, std::string> post_;   // POST 请求的数据
};

#endif