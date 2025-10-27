#include "buffer.h"

Buffer::Buffer(int init_buffer_size): buffer_(init_buffer_size), read_index_(0), write_index_(0){}

size_t Buffer::ReadableBytes() const {
    return write_index_ - read_index_;
}

size_t Buffer::WritableBytes() const {
    return buffer_.size() - write_index_;
}

size_t Buffer::PrependableBytes() const{
    return read_index_;
}

char* Buffer::WriteBegin(){
    return &buffer_[write_index_];
}

const char* Buffer::WriteBeginConst() const{
    return &buffer_[write_index_];
}

const char* Buffer::ReadBegin() const{
    return &buffer_[read_index_];
}

//确保缓冲区有足够的可写空间
void Buffer::EnsureWriteable(size_t len){
    if(len > WritableBytes()){
        MakeSpace(len);
    }
    assert(len <= WritableBytes());
}

//更新写指针
void Buffer::HasWritten(size_t len){
    // assert(len <= WritabelBytes());
    write_index_ += len;
}
//读取len长度，移动读下标
void Buffer::Retrieve(size_t len){
    // assert(len <= ReadableBytes());
    // read_index_ += len;
    if(len < ReadableBytes()){
        read_index_ += len;
    } else {
        RetrieveAll();
    }
}

void Buffer::RetrieveUntil(const char* end){
    assert(ReadBegin() < end);
    assert(end <= WriteBeginConst());
    Retrieve(end - ReadBegin());
}

// 取出所有数据
void Buffer::RetrieveAll() {
    read_index_ = write_index_ = 0;
}

// 以字符串的形式取出数据
std::string Buffer::RetrieveAsString(size_t len) {
    assert(len <= ReadableBytes());
    std::string str(ReadBegin(), len);
    Retrieve(len);
    return str;

}

// 以字符串的形式取出所有数据
std::string Buffer::RetrieveAllAsString() {
    return RetrieveAsString(ReadableBytes());
}


// 添加数据到buffer_中
void Buffer::Append(const char* str, size_t len) {
    assert(str);
    EnsureWriteable(len);
    std::copy(str, str + len, WriteBegin());
    HasWritten(len);
}
void Buffer::Append(const std::string& str) {
    Append(str.c_str(), str.size());
}
void Buffer::Append(const void* data, size_t len) {
    Append(static_cast<const char*>(data), len);
}
void Buffer::Append(const Buffer& buffer) {
    Append(buffer.ReadBegin(), buffer.ReadableBytes());
}




ssize_t Buffer::ReadFD(int fd, int* Errno){
    char buffer[65535];
    int writeable_bytes = WritableBytes();

    struct iovec iov[2];
    iov[0].iov_base = WriteBegin();
    iov[0].iov_len = writeable_bytes;
    iov[1].iov_base = &buffer;
    iov[1].iov_len = sizeof(buffer);

    ssize_t len = readv(fd, iov, 2);
    if(len < 0){
        *Errno = errno;
    } else if(static_cast<size_t>(len) <= static_cast<size_t>(writeable_bytes)){
        write_index_ += len;
    } else {
        write_index_ = buffer_.size();
        Append(buffer,  static_cast<size_t>(len) - writeable_bytes);
    }
    return len;
}


ssize_t Buffer::WriteFD(int fd, int* Errno){
    ssize_t len = write(fd, ReadBegin(), ReadableBytes());
    if(len < 0){
        *Errno = errno;
    }else{
        Retrieve(len);
    }
    return len;
}





char* Buffer::Begin(){
    return &buffer_[0];
}

const char* Buffer::Begin() const{
    return &buffer_[0];
}


void Buffer::MakeSpace(size_t len){
    if(len > WritableBytes() + PrependableBytes()){
        buffer_.resize(write_index_  + len);
    } else {
        size_t readable_bytes = ReadableBytes();
        std::copy(Begin()  + read_index_, Begin() + write_index_, Begin());
        read_index_ = 0;
        write_index_ = readable_bytes;
        assert(readable_bytes == ReadableBytes());
    }
}


