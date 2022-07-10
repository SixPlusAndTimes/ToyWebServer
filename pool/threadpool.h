//
// Created by 19269 on 2022/7/10.
//
#include <queue>
#include <functional>
#include <vector>
#include <thread>
#ifndef TOYWEBVSERVER_THREADPOOL_H
#define TOYWEBVSERVER_THREADPOOL_H

class threadPool
{
public:
    threadPool( int threadNum, int queueNum);
    ~threadPool();

    template<class T>
    void append(T &&task) {

    }

private:
    int threadNum;
    int queueNUm;
    std::queue<std::function<void()>> m_jobQueue;
    std::vector<std::thread> m_threadsQueue;
};

#endif //TOYWEBVSERVER_THREADPOOL_H
