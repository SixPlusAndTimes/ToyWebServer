//
// Created by 19269 on 2022/7/10.
//
#include <queue>
#include <functional>
#include <vector>
#include <thread>
#include "../lock/lock.h"
#ifndef TOYWEBVSERVER_THREADPOOL_H
#define TOYWEBVSERVER_THREADPOOL_H


//线程池相当于解一个生产者/消费者 问题
//我觉得可以当成无界缓冲区的情况

class threadpool
{
public:
    threadpool( int t_Num, int queueNum):
    threadNum(t_Num),jobQueueNum(queueNum) ,m_stop(false),threadPoolMutex(),threadPoolEmptyCond(){
        //创建t_Num个线程
        for(int i = 0; i < threadNum; i++) {
            //使用std::thread()创建线程，并执行lambda表达式里的内容
            //lambda表达式：小括号内是函数参数， ->后是返回类型；[]是函数对象参数，函数对象只能使用到定义该Lambda表达式为止定义过的局部变量
            //this：表示Lambda表达式可以使用Lambda表达式所在类的成员变量；
            std::thread([this]()->void
            {
                while(!m_stop)
                {
//                    printf("create %dth thread\n");
                    threadPoolMutex.lock();
                    while (m_jobQueue.empty()) {
                        threadPoolEmptyCond.wait(threadPoolMutex.getMutex());
                    }

                    auto todoTask = m_jobQueue.front();
                    m_jobQueue.pop();

                    threadPoolMutex.unlock();
//                    printf("one thread get the job\n");
                    todoTask();
                }
            }).detach();
        }
    }
    ~threadpool() {
        m_stop = true;
    };

    template<class T>
    bool append(T &&task) {
        threadPoolMutex.lock();
//        printf("getlock\n" );
        if(m_jobQueue.size() > jobQueueNum) {//这里还是要限制一下，不然会爆内存
            threadPoolMutex.unlock();
            return false;
        }
        m_jobQueue.emplace(std::forward<T>(task));
        threadPoolEmptyCond.signal();
        threadPoolMutex.unlock();
        return true;
    }
    threadpool( const threadpool &) = delete;
    threadpool &operator=(const threadpool &) = delete;
private:
    int threadNum;
    int jobQueueNum;//感觉工作队列可以不限制大小，否则只能返回失败，或者阻塞 append的线程（这不可能，因为执行append的是主线程，不能将主线程阻塞）
    std::queue<std::function<void()>> m_jobQueue;

    bool m_stop;//服务器是否停止

    mutex threadPoolMutex;
    cond threadPoolEmptyCond;
//    cond threadPoolFullCond; //无界缓冲区应该不需要第二个条件变量

};

#endif //TOYWEBVSERVER_THREADPOOL_H
