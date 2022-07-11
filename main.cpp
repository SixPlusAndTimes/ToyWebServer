//
// Created by 19269 on 2022/7/7.
//
//
#include <iostream>
#include "lock/lock.h"
#include "pool/threadpool.h"
#include "./epoll/epoller.h"
#include <pthread.h>

//#include <cstdlib>
int a = 0;
mutex mtx;
cond mcond;
void * work_increase(void *pVoid) {
    std::cout<<"work_increase\n";
    while(1) {
        mtx.lock();
        while(a == 1) {
            mcond.wait(mtx.getMutex());
        }
        a++;
        printf("increase a , a = %d \n",a);
        mcond.signal();
        mtx.unlock();
        printf("signal \n");
    }


}

void * word_decrease(void *pVoid) {
    std::cout<<"work_decrease\n";
    while(1) {
        mtx.lock();
        while(a == 0) {
            mcond.wait(mtx.getMutex());
        }
        a--;
        printf("increase a , a = %d \n",a);
        mcond.signal();
        mtx.unlock();
        printf("signal \n");
    }
}

void add(int a, int b) {
    printf("%d + %d = %d\n" , a , b, a + b );
}
int main() {

    threadpool threadPool(8,8);
//    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    while(1){
        bool ret = threadPool.append(std::bind(add, 1, 2)); 	// 向线程池中添加“任务”
        if(ret == false) {
//            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        }
    }
}
