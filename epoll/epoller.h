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
    bool del(int fd);

    int getSocktFd(int i) const;
    uint32_t getFdEvent(int i) const;
private:
    int m_epollFd;//使用 epoll_create，打开的一个epoll文件描述符
    int m_epollMaxEvents;//epoller能够监听的最大事件数目
    std::vector<struct epoll_event> m_epEves; //epoll_wait返回时，将触发的事件存储在这个vector中
};


#endif //TOYWEBVSERVER_EPOLLER_H
