#include "http_response.h"


const std::unordered_map<int, std::string> HttpResponse::CODE_STATUS = {
    {200, "OK"},
    {400, "Bad Requeset"},
    {403, "Forbidden"},
    {404, "Not Found"},
};

const std::unordered_map<int, std::string> HttpResponse::CODE_PATH = {
    {400, "/400.html"},
    {403, "/403.html"},
    {404, "/404.html"},
};

const std::unordered_map<std::string, std::string> HttpResponse::SUFFIX_TYPE = {
    {".html",  "text/html"},
    {".xml",   "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".txt",   "text/plain"},
    {".rtf",   "application/rtf"},
    {".pdf",   "application/pdf"},
    {".word",  "application/nsword"},
    {".png",   "image/png"},
    {".gif",   "image/gif"},
    {".jpg",   "image/jpeg"},
    {".jpeg",  "image/jpeg"},
    {".au",    "audio/basic"},
    {".mpeg",  "video/mpeg"},
    {".mpg",   "video/mpeg"},
    {".avi",   "video/x-msvideo"},
    {".gz",    "application/x-gzip"},
    {".tar",   "application/x-tar"},
    {".css",   "text/css"},
    {".js",    "text/javascript"},
};


HttpResponse::HttpResponse(): code_(-1), is_keep_alive_(false), path_(""), src_dir_(""), mmfile_(nullptr){
    memset(&mmfile_stat_, 0, sizeof(mmfile_stat_));
}

HttpResponse::~HttpResponse(){
    UnmapFile();
}


void HttpResponse::Init(const std::string& path, const std::string& src_dir, int code, bool is_keep_alive){
    assert(src_dir != ""); //资源目录不能为空
    code_ = code;
    is_keep_alive_ = is_keep_alive;
    path_ = path;
    src_dir_ = src_dir;
    mmfile_ = nullptr;
    memset(&mmfile_stat_, 0, sizeof(mmfile_stat_));
}

// 解除内存映射
void HttpResponse::UnmapFile(){
    if(mmfile_){
        munmap(mmfile_, mmfile_stat_.st_size);
        mmfile_ = nullptr;
    }
}

void HttpResponse::ErrorHtml(){
    if(CODE_PATH.count(code_) == 1){
        path_ = CODE_PATH.find(code_)->second;
        stat((src_dir_ + path_).c_str(), &mmfile_stat_);
    }
}

void HttpResponse::AddStateLine(Buffer& buff){
    std::string status;
    if(CODE_STATUS.count(code_) == 1){
        status = CODE_STATUS.find(code_)->second;
    } else {
        code_ = 400;
        status = CODE_STATUS.find(400)->second;
    }
    buff.Append("HTTP/1.1 " + std::to_string(code_) + " " + status + "\r\n");
}


void HttpResponse::AddHeader(Buffer& buff){
    buff.Append("Connection: ");
    if(is_keep_alive_){
        buff.Append("keep-alive\r\n");
        buff.Append("Keep-Alive: timeout=120\r\n");
    } else {
        buff.Append("close\r\n");
    }
    buff.Append("Content-type: " + GetFileType() + "\r\n");
}

std::string HttpResponse::GetFileType(){
    std::string::size_type idx = path_.find_last_of('.');
    if(idx == std::string::npos){
        return "text/plain";
    }
    std::string suffix = path_.substr(idx);
    if(SUFFIX_TYPE.count(suffix) == 1){
        return SUFFIX_TYPE.find(suffix)->second;
    }
    return "text/plain";
}

// 将文件映射进内存地址中
void HttpResponse::AddContent(Buffer& buff){
    std::string path = src_dir_ + path_;
    int src_fd = open(path.c_str(), O_RDONLY);
    if(src_fd < 0){
        ErrorContent(buff, "File Not Found!");
        return;
    }
    LOG_DEBUG("File path: %s", path.c_str());
    //获取文件状态
    void* mmret = mmap(0, mmfile_stat_.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
    if(mmret == MAP_FAILED){
        close(src_fd);
        ErrorContent(buff, "File Not Found!");
        return;
    }
    mmfile_ = static_cast<char*>(mmret);
    close(src_fd);
    buff.Append("Content-length: " + std::to_string(mmfile_stat_.st_size) + "\r\n\r\n");
}


void HttpResponse::ErrorContent(Buffer& buff, const std::string& message){
    std::string body;
    std::string status;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    if(CODE_STATUS.count(code_) == 1){
        status = CODE_STATUS.find(code_)->second;
    } else {   
        status = "Bad Request";
    }
    body += std::to_string(code_) + " : " + status + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>WebServer</em></body></html>";

    buff.Append("Content-length: " + std::to_string(body.size()) + "\r\n\r\n");
    buff.Append(body);
}

void HttpResponse::MakeResponse(Buffer& buff){
    if (stat((src_dir_ + path_).c_str(), &mmfile_stat_) < 0) {
        LOG_WARN("stat fail: error: %s", strerror(errno));
        code_ = 404;
    } else if (S_ISDIR(mmfile_stat_.st_mode)) {
        code_ = 404;
    } else if (!(mmfile_stat_.st_mode & S_IROTH)) {
        code_ = 403;
    } else if (code_ == -1) {
        code_ = 200;
    }
    ErrorHtml();
    AddStateLine(buff);
    AddHeader(buff);
    AddContent(buff);
}