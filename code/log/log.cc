#include "log.h"

Log::Log() 
    : is_async_(false), today_(0),
      line_count_(0), fp_(nullptr),
      deque_(nullptr), write_thread_(nullptr) {}

Log::~Log() {
    while (!deque_->empty())
        deque_->flush(); // 唤醒消费者，处理剩下数据
    deque_->close();
    write_thread_->join(); // 等待线程退出
    if (fp_) {
        std::lock_guard<std::mutex> locker(mtx_);
        Flush();
        fclose(fp_);
    }
}

// 初始化
void Log::Init(int level, const char* path,
              const char* suffix, 
              int max_capacity) {
    is_open_ = true;
    level_ = level; 
    path_ = path;
    suffix_ = suffix;

    if (max_capacity) { // 异步
        is_async_ = true;
        if (!deque_) {
            std::unique_ptr<BlockQueue<string>> new_deque(new BlockQueue<string>);
            deque_ = std::move(new_deque); // 所有权转移
            std::unique_ptr<thread> new_thread(new thread(FLushLogThread));
            write_thread_ = std::move(new_thread);
        }
    } else { // 同步
        is_async_ = false;
    }

    line_count_ = 0;

    path_ = path;
    suffix_ = suffix;
    time_t timer = time(nullptr);
    struct tm* sys_time = localtime(&timer);
    struct tm t = *sys_time;
    char filename[Log_NAME_LENGTH] = {0};
    snprintf(filename, Log_NAME_LENGTH - 1, "%s%04d_%02d_%0d%s",
             path_, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, suffix_);
    today_ = t.tm_mday;

    {
        std::lock_guard<std::mutex> locker(mtx_);
        if (fp_) {
            Flush(); 
            fclose(fp_);
        }
        fp_ = fopen(filename, "a");
        if (fp_ == nullptr) {
            mkdir(path_, 0777); // 777最大权限
            fp_ = fopen(filename, "a");
        }
        assert(fp_ != nullptr);
    }
}

void Log::AppendLogLevel(int level) {
    const char* level_title[] = {"[DEBUG]: ", "[INFO] : ", "[WARN] : ",
                                "[ERROR]: ", "[FATAL]: "};
    int valid_level = (level >= 0 && level <= 4) ? level : 1;
    buff_.Append(level_title[valid_level], 9);
}

void Log::Write(int level, const char* format, ...) {
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);
    time_t time_second =  now.tv_sec;
    struct tm* sys_time = localtime(&time_second);
    struct tm t = *sys_time;

    // 日期不对或行数满了
    if (today_ != t.tm_mday || (line_count_ && (line_count_ % MAX_LINES == 0))) {
        std::unique_lock<std::mutex> locker(mtx_);
        locker.unlock();

        char new_file[Log_NAME_LENGTH];
        char tail[36];
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

        if (today_ != t.tm_mday) {
            snprintf(new_file, Log_NAME_LENGTH - 72, "%s%s%s", path_, tail, suffix_);
            today_ = t.tm_mday;
        } else {
            int num = line_count_ / MAX_LINES;
            snprintf(new_file, Log_NAME_LENGTH, "%s%s-%d%s", path_, tail, num, suffix_);
        }

        {
            std::lock_guard<std::mutex> locker(mtx_);
            Flush();
            fclose(fp_);
            fp_ = fopen(new_file, "a");
            assert(fp_ != nullptr);
        }
    }

    {
        std::lock_guard<std::mutex> locker(mtx_);
        line_count_++;

        int n = snprintf(buff_.WriteBegin(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",     
                         t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, 
                         t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);
        buff_.HasWritten(n);
        AppendLogLevel(level);

        va_list vaList;
        va_start(vaList, format);
        int m = vsnprintf(buff_.WriteBegin(), buff_.WritableBytes(), format, vaList);
        va_end(vaList);
        buff_.HasWritten(m);
        buff_.Append("\n\0", 2);

        if (is_async_ && deque_) // 异步模式-生产者
            deque_->push_back(buff_.RetrieveAllAsString());
        else // 同步模式-直接写入
            fputs(buff_.ReadBegin(), fp_);    
        buff_.RetrieveAll(); 
    }
}

// 单例模式之饿汉模式
Log* Log::GetInstance() {
    // 静态局部变量的初始化是线程安全的
    static Log log;
    return &log;
}

// 异步日志的写线程函数
void Log::FLushLogThread() {
    Log::GetInstance()->AsyncWrite();
}

// 写线程真正的执行函数
void Log::AsyncWrite() {
    string str = "";
    while (deque_->pop(str)) { // 异步模式-消费者
        std::lock_guard<std::mutex> locker(mtx_);
        fputs(str.c_str(), fp_);
    }
}

// 唤醒消费者，开始写日志
void Log::Flush() {
    if (is_async_)
        deque_->flush();
    fflush(fp_);
}

int Log::GetLevel() {
    std::lock_guard<std::mutex> lock(mtx_);
    return level_;
}

void Log::SetLevel(int level) {
    std::lock_guard<std::mutex> lock(mtx_);
    level_ = level;
}

bool Log::IsOpen() {
    return is_open_;
}