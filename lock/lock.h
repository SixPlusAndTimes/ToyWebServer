//
// Created by 19269 on 2022/7/10.
//
#include <pthread.h>
#include <exception>
#include <stdexcept>
#ifndef TOYWEBVSERVER_LOCK_H
#define TOYWEBVSERVER_LOCK_H

class mutex
{
public:
    mutex() {
        if(pthread_mutex_init(&m_mutex,NULL) != 0) {
            throw std::runtime_error("pthread_init() failed");
        }

    }
    ~mutex() {
        pthread_mutex_destroy(&m_mutex);
    }

    bool lock() {
        return pthread_mutex_lock(&m_mutex) == 0;
    }

    bool unlock() {
        return pthread_mutex_unlock(&m_mutex) == 0;
    }

    pthread_mutex_t* getMutex() {
        return &m_mutex;
    }


private:
    pthread_mutex_t  m_mutex;
};

class cond
{
public:
    cond() {
        if(pthread_cond_init(&m_cond,NULL) != 0) {
            throw std::runtime_error("pthread_cond_init() failed!");
        }
    }

    ~cond(){
        pthread_cond_destroy(&m_cond);
    }

    bool wait(pthread_mutex_t* mutex) {
        return pthread_cond_wait(&m_cond,mutex) == 0;
    }

    bool signal() {
        return pthread_cond_signal(&m_cond) == 0;
    }

    bool broadCast() {
        return pthread_cond_broadcast(&m_cond) == 0;
    }

private:
    pthread_cond_t m_cond;
};




#endif //TOYWEBVSERVER_LOCK_H
