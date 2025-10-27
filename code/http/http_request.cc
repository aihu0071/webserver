#include "http_request.h"

const std::unordered_set<std::string> HttpRequest::DEFAULT_HTML{
    "/index", "/register", "/login", "/welcome",
    "/video", "/picture"
};

const std::unordered_map<std::string, int> HttpRequest::DEFAULT_HTML_TAG {
    {"/login.html", 1}, {"/register.html", 0}
};

void HttpRequest::Init() {
    state_ = REQUEST_LINE;
    method_ = "";
    path_ = "";
    version_ = "";
    body_ = "";
    header_.clear();
    post_.clear();
    
}

bool HttpRequest::ParseRequestLine(const std::string& line){
    // GET /index.html HTTP/1.1
    std::regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch match;
    if (std::regex_match(line, match, patten)) {
        method_ = match[1];
        path_ = match[2];
        version_ = match[3];
        state_ = HEADERS;
        return true;
    } 
    LOG_ERROR("RequestLine Error"); 
    return false;
}

void HttpRequest::ParseHeader(const std::string& line){
    // Host: localhost:8080
    std::regex patten("^([^:]*): ?(.*)$");
    std::smatch match;
    if (std::regex_match(line, match, patten)) {
        header_[match[1]] = match[2];
    } else {
        state_ = BODY;
    }
}

void HttpRequest::ParseBody(const std::string& line) {
    body_ = line;
    ParsePost(); 
    state_ = FINISH;
    LOG_DEBUG("Body: %s, len: %d", line.c_str(), line.size());
}

void HttpRequest::ParsePost() {
    if (method_ == "POST" && header_["Content-Type"] == "application/x-www-form-urlencoded") {
        ParseFromUrlEncoded();
        if (DEFAULT_HTML_TAG.count(path_)) { // 登录/注册
            int tag = DEFAULT_HTML_TAG.find(path_)->second;
            LOG_DEBUG("Tag: %d", tag);
            if (tag == 0 || tag == 1) {
                bool is_login = (tag == 1);
                if (UserVerify(post_["username"], post_["password"], is_login))
                    path_ = "/welcome.html";
                else
                    path_ = "/error.html";
            }
        }
    }
}

void HttpRequest::ParsePath() {
    if (path_ == "/") {
        path_ = "/index.html";
    } else {
        if (DEFAULT_HTML.find(path_) != DEFAULT_HTML.end())
            path_ += ".html";
    }
}

int HttpRequest::ConverHex(char ch) {
    if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    return ch;
}


void HttpRequest::ParseFromUrlEncoded(){
    if(body_.size() == 0){
        return;
    }
    std::string key, value;
    int num = 0;
    int n = body_.size();
    int i = 0, j = 0;
    // username=john%20doe&password=123456
    while(i < n){
        char ch = body_[i];
        switch (ch) {
        case '=': // 获取键值对
            key = body_.substr(j, i - j); 
            j = i + 1;
            break;
        case '&': // 获取键值对
            value = body_.substr(j, i - j);
            j = i + 1;
            post_[key] = value;
            LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
            break;
        case '+': // 替换为空格
            body_[i] = ' ';
            break;
        case '%':
            num = ConverHex(body_[i + 1]) * 16 + ConverHex(body_[i + 2]);
            body_[i + 2] = num % 10 + '0';
            body_[i + 1] = num / 10 + '0';
            i += 2;
            break;
        default:
            break;
        }
        i++;
    }
    assert(j <= i);
    if (post_.count(key) == 0 && j < i) {
        value = body_.substr(j, i - j);
        post_[key] = value;
    }
}

bool HttpRequest::UserVerify(const std::string& name, const std::string& pwd, bool is_login) {
    if(name == "" || pwd == ""){
        return false;
    }
    LOG_INFO("Verify user: %s with pwd: %s", name.c_str(), pwd.c_str());
    MYSQL* sql;
    SqlConnectRAII(&sql, SqlConnectPool::instance());
    assert(sql);

    bool flag = false;
    char order[256] = {0};
    MYSQL_RES* res = nullptr;      // 查询结果集
    if (!is_login) // 注册
        flag = true;
    snprintf(order, 256, "SELECT username, password FROM User WHERE username='%s' LIMIT 1", name.c_str());
    LOG_DEBUG("%s", order);

    // 查询用户信息
    if (mysql_query(sql, order)) {
        if (res)
            mysql_free_result(res); // 查询失败，释放结果集
        return false;
    }
    res = mysql_store_result(sql);   // 存储查询结果到res中

    // 处理查询结果
    while (MYSQL_ROW row = mysql_fetch_row(res)) {
        LOG_DEBUG("MYSQL ROW: %s %s", row[0], row[1]);
        std::string password = row[1];
        if (is_login) {
            if (pwd == password) {
                flag = true;
            } else {
                flag = false;
                LOG_INFO("pwd error!");
            }
        } else {
            flag = false;
            LOG_INFO("user used!");
        }
    }
    mysql_free_result(res);
    // 注册（用户名未被使用）
    if (!is_login && flag) {
        LOG_DEBUG("register");
        bzero(order, 256);
        snprintf(order, 256, "INSERT INTO User(username, password) VALUES('%s', '%s')", name.c_str(), pwd.c_str());
        LOG_DEBUG("%s", order);
        if (mysql_query(sql, order)) {
            LOG_ERROR("MySQL insert fail: Error: %s", mysql_error(sql));
            flag = false;
        } else {
            LOG_DEBUG("UserVerify success!");
            flag = true;
        }
    }
    return flag;
}

// 解析 HTTP 请求
bool HttpRequest::Parse(Buffer& buff) {
    const char* END = "\r\n";
    if (buff.ReadableBytes() == 0)
        return false;
    
    while (buff.ReadableBytes() && state_ != FINISH) {
        // 找到buff中，首次出现"\r\n"的位置
        const char* line_end = std::search(buff.ReadBegin(), buff.WriteBeginConst(), END, END + 2);
        std::string line(buff.ReadBegin(), line_end);
        switch (state_) {
        case REQUEST_LINE:
            if (ParseRequestLine(line) == false)
                return false;
            ParsePath();
            break;
        case HEADERS:
            ParseHeader(line);
            if (buff.ReadableBytes() <= 2) // get请求，提前结束
                state_ = FINISH;
            break;
        case BODY:
            ParseBody(line);
            break;
        default:
            break;
        }
        if (line_end == buff.WriteBegin()) { // 读完了
            buff.RetrieveAll();
            break;
        }
        buff.RetrieveUntil(line_end + 2); // 跳过回车换行
    }
    LOG_DEBUG("[%s] [%s] [%s]", method_ .c_str(), path_.c_str(), version_.c_str());
    return true;
}

std::string HttpRequest::Method() const {
    return method_;
}

std::string HttpRequest::Path() const {
    return path_;
}

std::string& HttpRequest::Path() {
    return path_;
}

std::string HttpRequest::Version() const {
    return version_;
}

std::string HttpRequest::GetPost(const std::string& key) const {
    if (post_.count(key) == 1)
        // return post_[key]; post_有const属性，所以不能用[]
        return post_.find(key)->second;
    return "";
}

std::string HttpRequest::GetPost(const char* key) const {
    assert(key != nullptr);
    if (post_.count(key) == 1)
        return post_.find(key)->second;
    return "";
}

bool HttpRequest::IsKeepAlive() const {
    if (header_.count("Connect") == 1) 
        return header_.find("Connect")->second == "keep-alive" && version_ == "1.1";
    return false;
}
