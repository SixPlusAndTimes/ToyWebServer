//
// Created by 19269 on 2022/7/14.
//

#ifndef TOYWEBVSERVER_HTTPCONNECT_H
#define TOYWEBVSERVER_HTTPCONNECT_H

#include <arpa/inet.h> //sockaddr_in
#include <sys/uio.h>   //readv/writev
#include <iostream>
#include <sys/types.h>
#include <assert.h>
#include <atomic> // std::atomic<int>
//#include "httprequest.h"
//#include "httpresponse.h"
#include "../buffer/buffer.h"
class Httpconnection {
public:
    Httpconnection();


    ~Httpconnection();

    /*
     *  socketFd : accept产生的fd， sockaddr_in 对端客户端的地址
     * */
    void initHTTPConn(int socketFd, const sockaddr_in &addr);

    //每个连接中定义的对缓冲区的读写接口
    ssize_t readBuffer(int *saveErrno);

    ssize_t writeBuffer(int *saveErrno);

    //关闭HTTP连接的接口
    void closeHTTPConn();

    //定义处理该HTTP连接的接口，主要分为request的解析和response的生成
    bool handleHTTPConn();

    //其他方法
    const char *getIP() const;

    int getPort() const;

    int getFd() const;

    sockaddr_in getAddr() const;

    //返回写出去的字节数：m_iov[0].iov_len + m_iov[1].iov_len;
    int writeBytes();

    bool isKeepAlive() const;

    bool isGetRightResource;
    bool isHttpDone;

    // webserver的initTrigMode会再次对该属性设值
    static bool isET; //是否所有的HTTP连接都是ET模式
    static const char *srcDir; // 所有HTTP连接的资源根路径
    static std::atomic<int> userCount; //一共由多少个http连接，因为会被多个线程修改，因此需要原子类

    // 用于统计程序切入切出间隔时间的计时
    struct timeval m_eplR1;
    struct timeval m_eplR2;
    struct timeval m_Thre1;
    struct timeval m_Thre2;
    struct timeval m_eplW1;
    struct timeval m_eplW2;

private:
    int m_fd; //对应的一个socket描述符，由accept产生的描述符
    struct sockaddr_in m_addr;
    bool isClose; //

    int m_iovCnt;
    struct iovec m_iov[2]; // 用于分散写writev的结构体



    Buffer httpReadBuf;
    Buffer httpWriteBuf;
//
//    Httprequest m_request;
//    Httpresponse m_response;
};

#endif //TOYWEBVSERVER_HTTPCONNECT_H
