//
// Created by 19269 on 2022/7/11.
//
#include <unistd.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <vector>
#ifndef TOYWEBVSERVER_EPOLLER_H
#define TOYWEBVSERVER_EPOLLER_H

class Epoller
{
public:
    Epoller(int maxEventNums = 1024);
    ~Epoller() = default;
    int wait(int timeout = -1);
    bool add(int fd, uint32_t ev);
    bool mod(int fd, uint32_t ev);
    bool del(int fd, uint32_t ev);

    int getSocktFd(int i) const;
    int getEventFd() const;
private:
    int m_epillFd;

    std::vector<struct epoll_event> m_epEves;
};


#endif //TOYWEBVSERVER_EPOLLER_H
