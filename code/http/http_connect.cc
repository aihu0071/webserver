#include "http_connect.h"

bool HttpConnect::is_ET;
const char* HttpConnect::src_dir;
std::atomic<int> HttpConnect::use_count;

HttpConnect::HttpConnect()
    : fd_(-1), is_close_(true) {
    memset(&addr_, 0, sizeof(addr_));
}

HttpConnect::~HttpConnect() {
    Close();
}



void HttpConnect::Init(int socket_fd, const sockaddr_in& addr){
    assert(socket_fd > 0);
    ++use_count;
    fd_ = socket_fd;
    addr_ = addr;
    read_buff_.RetrieveAll();
    write_buff_.RetrieveAll();
    LOG_INFO("Client[%d][%s:%d] in, user count: %d", fd_, GetIP(), GetPort(), (int)use_count);
}

void HttpConnect::Close(){
    response_.UnmapFile();
    if(!is_close_){
        is_close_ = true;
        --use_count;
        close(fd_);
        LOG_INFO("Client[%d][%s:%d] quit, user count: %d", fd_, GetIP(), GetPort(), (int)use_count);
    }
}

ssize_t HttpConnect::Read(int* save_errno) {
    ssize_t len = -1;
    do {
        len = read_buff_.ReadFD(fd_, save_errno);
        if (len <= 0)
            break;
    } while (is_ET); // EF: 边沿触发，循环读取数据将数据处理完
    return len;
}


ssize_t HttpConnect::Write(int* save_errno) {
    ssize_t len = -1;
    do {
        len = writev(fd_, iov_, iov_cnt_);
        if (len <= 0) {
            *save_errno = errno;
            break;
        }

        if (iov_[0].iov_len + iov_[1].iov_len == 0) { // 读完
            break;
        } else if (static_cast<size_t>(len) > iov_[0].iov_len) { // iov[0]读完
            // 更新指针和长度
            size_t read_len = len - iov_[0].iov_len;
            iov_[1].iov_base = (uint8_t*)iov_[1].iov_base + read_len;
            iov_[1].iov_len -= read_len;
            if (iov_[0].iov_len) {
                write_buff_.RetrieveAll();
                iov_[0].iov_len = 0;
            }
        } else { // iov[0]未读完
            iov_[0].iov_base = (uint8_t*)iov_[0].iov_base + len;
            iov_[0].iov_len -= len;
            write_buff_.Retrieve(len);
        }
    } while (is_ET || ToWriteBytes() > 10240);
    // 边沿触发模式下数据被全部写入
    // 在非边沿触发模式下，或需要写入的数据量较大时，也能分多次写入
    return len;
}


bool HttpConnect::Process() {
    request_.Init();
    if (read_buff_.ReadableBytes() <= 0)
        return false;
    if (request_.Parse(read_buff_)) {
        LOG_DEBUG("path: %s", request_.Path().c_str());
        response_.Init(request_.Path(), src_dir, 200, request_.IsKeepAlive());
    } else {
        response_.Init(request_.Path(), src_dir, 400, false);
    }

    response_.MakeResponse(write_buff_);
    iov_[0].iov_base = const_cast<char*>(write_buff_.ReadBegin());
    iov_[0].iov_len = write_buff_.ReadableBytes();
    iov_cnt_ = 1;

    // 报文主体文件
    if (response_.File()) {
        iov_[1].iov_base = response_.File();
        iov_[1].iov_len = response_.FileLen();
        iov_cnt_ = 2;
    }
    LOG_DEBUG("filesize: %d, %d to %d", response_.FileLen(), iov_cnt_, ToWriteBytes());
    return true;
}
