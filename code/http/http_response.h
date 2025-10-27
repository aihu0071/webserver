#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H


#include <sys/stat.h> // stat
#include <sys/mman.h> // mmap, munmap
#include <fcntl.h>    // open
#include <unistd.h>   // close

#include <cstring>    // memset
#include <cassert>
#include <string>
#include <unordered_map>

#include "../buffer/buffer.h"
#include "../log/log.h"

class HttpResponse{
public:
    HttpResponse();
    ~HttpResponse();
    void Init(const std::string& path, const std::string& src_dir, int code = -1, bool is_keep_alive = false);
    void UnmapFile();   // 解除内存映射

    void MakeResponse(Buffer& buff);
    void ErrorContent(Buffer& buff, const std::string& message);

    char* File() { return mmfile_; }
    size_t FileLen() const { return mmfile_stat_.st_size; }
    int Code() const { return code_; }
private:
    void AddStateLine(Buffer& buff);
    void AddHeader(Buffer& buff);
    void AddContent(Buffer& buff);

    void ErrorHtml();
    std::string GetFileType();

    static const std::unordered_map<int, std::string> CODE_STATUS;          // 编码状态集
    static const std::unordered_map<int, std::string> CODE_PATH;            // 编码路径集
    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;  // 后缀状态集

    int code_;
    bool is_keep_alive_; //是否保持连接

    std::string path_; //请求路径
    std::string src_dir_; //资源目录

    char* mmfile_; //内存映射地址
    struct stat mmfile_stat_; //文件状态    
};


#endif