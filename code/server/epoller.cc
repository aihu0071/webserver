#include "epoller.h"

Epoller::Epoller(int max_event) 
    : epoll_fd_(epoll_create(512)), events_(max_event) {
    assert(epoll_fd_ >= 0 && events_.size() > 0); 
}

Epoller::~Epoller() {
    close(epoll_fd_);
}

bool Epoller::AddFd(int fd, uint32_t events) {
    if (fd < 0)
        return false;
    epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));
    ev.data.fd = fd;
    ev.events = events;
    return epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == 0;
}

bool Epoller::ModFd(int fd, uint32_t events) {
    if (fd < 0)
        return false;
    epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));
    ev.data.fd = fd;
    ev.events = events;
    return epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) == 0;
}

bool Epoller::DelFd(int fd) {
    if (fd < 0)
        return false;
    return epoll_ctl(fd, EPOLL_CTL_DEL, fd, 0) == 0;
}

int Epoller::Wait(int timeout_ms) {
    return epoll_wait(epoll_fd_, &events_[0], static_cast<int>(events_.size()), timeout_ms);
}

int Epoller::GetEventsFd(size_t i) const {
    assert(i < events_.size());
    return events_[i].data.fd;
}

uint32_t Epoller::GetEvents(size_t i) const {
    assert(i < events_.size());
    return events_[i].events;
}