//
// Created by 19269 on 2022/7/11.
//
/*
 * epoll 系统调用相关
   struct epoll_event {
       uint32_t事件； / * Epoll事件* /
       epoll_data_t数据； / *用户数据变量* /
   };
    typedef union epoll_data {
               无效* ptr;
               int fd;
               uint32_t u32;
               uint64_t u64;
           } epoll_data_t;
 * */

/*
#include <sys / epoll.h>
int epoll_wait（int epfd，struct epoll_event * events， int maxevents，int timeout）;
 指定超时值timeout为-1会导致epoll_wait（）无限期阻塞，而指定的超时时间等于零导致epoll_wait（）立即返回，即使没有可用事件。
 * */
#include <iostream>
#include "epoller.h"
Epoller::Epoller(int maxEventNums)
    : m_epollFd(epoll_create(1)),m_epollMaxEvents(maxEventNums),m_epEves(maxEventNums) {}

int Epoller::wait(int timeout) {
//    std::cout << "timeWait...\n";
    //为什么&m_epEves不行，必须&m_epEves[0]
    //std::vector 可能和数组不太一样
    int nums = epoll_wait(m_epollFd, &m_epEves[0],m_epollMaxEvents,timeout);//timeout默认为-1，即epollwait将一致阻塞
//    std::cout << "timeWait return \n";
    return nums;
}
/*fd: socket描述符
 * ev: 事件属性： 是ET还是LT？是读时间还是写事件*/
bool Epoller::add(int fd, uint32_t ev)
{
    if(fd < 0)
    {
        return  false;
    }
    struct  epoll_event event;
    event.events = ev;
    event.data.fd = fd;
    epoll_ctl(m_epollFd,EPOLL_CTL_ADD, fd,&event);
//    std::cout << "add an fd\n";
    return true;
}

bool Epoller::del(int fd)
{
    if (fd < 0)
    {
        return false;
    }
    epoll_ctl(m_epollFd, EPOLL_CTL_DEL, fd, NULL);
    return true;
}

bool Epoller::mod(int fd, uint32_t ev)
{
    if (fd < 0)
    {
        return false;
    }
    struct epoll_event event;
    event.events = ev;
    event.data.fd = fd;
    epoll_ctl(m_epollFd, EPOLL_CTL_MOD, fd, &event);
    return true;
}


uint32_t Epoller::getFdEvent(int i) const
{
    if(i < 0 || i > m_epollMaxEvents)
    {
        return false;
    }
    return m_epEves[i].events;
}

int Epoller::getSocktFd(int i) const
{
    if(i < 0 || i > m_epollMaxEvents) {
        return false;
    }
    return m_epEves[i].data.fd;
}