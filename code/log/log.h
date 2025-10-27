#ifndef Log_H
#define Log_H


#include <sys/stat.h> // mkdir
#include <sys/time.h> // gettimeofday
#include <cstdio>   // FILE
#include <cstdarg>  // va_start
#include <ctime>
#include <cassert>
#include <string>
#include <utility>  // move
#include <memory>   // unique_ptr
#include <thread>

#include "blockqueue.h"
#include "../buffer/buffer.h"

using std::string;
using std::thread;

class Log{
public:
    void Init(int level, const char* path = "./log", 
              const char* suffix = ".log", 
              int max_capacity = 1024);

    static Log* GetInstance();
    static void FLushLogThread();  

    void Flush();
    void Write(int level, const char* format, ...);

    int GetLevel();
    void SetLevel(int level);
    bool IsOpen();

private:
    Log();
    ~Log();

    void AppendLogLevel(int level);
    void AsyncWrite();

    static const int Log_NAME_LENGTH = 256; //日志文件名最大长度
    static const int MAX_LINES = 50000; //日志文件最大行数

    const char* path_; //日志文件路径
    const char* suffix_; //日志文件后缀

    bool is_open_; //日志是否打开
    int level_; //日志等级
    bool is_async_; //是否异步写日志

    int today_;      //记录当前是哪一天
    int line_count_; //记录当前日志文件的行数

    Buffer buff_;  //日志缓冲区
    FILE* fp_;     //日志文件指针

    std::mutex mtx_; //互斥锁
    std::unique_ptr<BlockQueue<string>> deque_; //日志阻塞队列
    std::unique_ptr<thread> write_thread_; //日志写入线程
    
};

#define LOG_BASE(level, format, ...) \
    do { \
        Log* log = Log::GetInstance(); \
        if (log->IsOpen() && log->GetLevel() <= level) { \
            log->Write(level, format, ##__VA_ARGS__); \
            log->Flush(); \
        } \
    } while(0);

#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);
#define LOG_FATAL(format, ...) do {LOG_BASE(4, format, ##__VA_ARGS__)} while(0);


#endif // LOG_H