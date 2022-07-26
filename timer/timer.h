//
// Created by 19269 on 2022/7/26.
//

#ifndef TOYWEBVSERVER_TIMER_H
#define TOYWEBVSERVER_TIMER_H
#include <chrono>
#include <functional>
#include <memory>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <iostream>
#include "../log/log.h"
#include "../lock/lock.h"


typedef std::chrono::milliseconds _ms;
typedef std::chrono::high_resolution_clock _clock;
typedef _clock::time_point _timeStamp;
typedef std::function<void()> _timeoutCallBack;


//最小堆实现定时器的管理
class Timer {
public:
    Timer() = default;
    Timer(int fd, const _timeStamp expire, const _timeoutCallBack& func): so_fd_(fd), expire_(expire), timeout_callback_(func) {}
    ~Timer() = default;
    bool operator<(const Timer &t) {
        return expire_ < t.expire_;
    }

    bool operator>(const Timer &t) {
        return expire_ > t.expire_;
    }

    Timer &operator=(const Timer &t)
    {
        // LOG_DEBUG("正在执行赋值拷贝构造函数");
        so_fd_ = t.so_fd_;
        timeout_callback_ = t.timeout_callback_;
        expire_ = t.expire_;
        return *this;
    }

public:
    int so_fd_;
    _timeStamp expire_;
    _timeoutCallBack timeout_callback_;
};


class TimerManager {
public:
    TimerManager() = default;
    ~TimerManager() = default;
    //根据fd有去重逻辑
    void addTimer(int fd, int timeout , const _timeoutCallBack &func);

    //首先调用handleExpireEvent()将到期的timer的回调函数全部执行
        //返回值： 然后返回heap中的最顶端timer距离它的过期时间还有多长时间
    int getNextHandle();
    void updateTimer(int fd, int timeout);

    //删除指定的fd， 需要哈希表来辅助降低时间复杂度;
    void delFd(int fd);

    void work(int id);
    void pop();
    void clear();
private:
    //删除堆中的第i个元素
    void m_del(int i);
    //将节点i的timer上移，直到比父节点的值大
    void swim(int index );
    //将节点i的timer下移，直到找到一个合适的位置；n代表能够下移的界限
    bool sink(int i, int n);
    void swapNode(int i, int j );

    void handleExpireEvent();

    //这两个锁没用到，因为只有主线程在执行webserver.cpp的代码时，才会操做这个类
    //单线程，不用考虑线程安全维问题
//    mutex heap_mutex_;
//    mutex hash_table_mutex_;


    // 最小堆想当于用一个数组来表示
    // 最小堆是一种完全二叉树，可以用数组来组织其中的元素
    std::vector<Timer> heap_;

    std::unordered_map<int, int> fd_id_map_; // 映射，fd -> id(index in heap)
};

#endif //TOYWEBVSERVER_TIMER_H
