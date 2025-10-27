#ifndef BUFFER_H
#define BUFFER_H

#include <unistd.h>
#include <sys/uio.h>   // readv/writev

#include <iostream>
#include <vector>
#include <string>
#include <atomic>
#include <cassert>


class Buffer{
public:
    Buffer(int init_buffer_size = 1024);
    ~Buffer() = default;

    //获取字节大小函数
    size_t ReadableBytes() const;
    size_t WritableBytes() const;
    size_t PrependableBytes() const;

    char* WriteBegin();
    const char* WriteBeginConst() const;
    const char* ReadBegin() const;

    void EnsureWriteable(size_t len);
    void HasWritten(size_t len);
    void Retrieve(size_t len);

    void RetrieveUntil(const char* end);
    void RetrieveAll();

    std::string RetrieveAsString(size_t len);
    std::string RetrieveAllAsString();

    //添加数据函数
    void Append(const char* str, size_t len);
    void Append(const std::string& str);
    void Append(const void* data, size_t len);
    void Append(const Buffer& buffer);


    ssize_t ReadFD(int fd, int* Errno);
    ssize_t WriteFD(int fd, int* Errno);

private:
    //获取缓冲区起始地址
    char* Begin();
    const char* Begin() const;
    
    //确保足够的内部空间
    void MakeSpace(size_t len);

    std::vector<char> buffer_;
    std::atomic<size_t> read_index_;
    std::atomic<size_t> write_index_;
    
};

#endif 