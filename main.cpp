//
// Created by 19269 on 2022/7/7.
//
#include <iostream>
#include "lock/lock.h"
#include <pthread.h>
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


int main() {
    pthread_t  t1, t2;
    pthread_create(&t1,NULL,work_increase, NULL);
    pthread_create(&t1,NULL,word_decrease, NULL);
    std::cout<<"main\n";
    while(1) {

    }
}
