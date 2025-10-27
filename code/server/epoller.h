#ifndef EPOLLER_H
#define EPOLLER_H

#include <unistd.h>
#include <sys/epoll.h>
#include <cassert>
#include <cstring>
#include <vector>

class Epoller {
public:
    explicit Epoller(int max_event = 1024);
    ~Epoller();

    bool AddFd(int fd, uint32_t events);
    bool ModFd(int fd, uint32_t events);
    bool DelFd(int fd);
    int Wait(int timeout_ms = -1);
    int GetEventsFd(size_t i) const;
    uint32_t GetEvents(size_t i) const;

private:
    int epoll_fd_;
    std::vector<struct epoll_event> events_;
};

#endif // EPOOLER_H