//
// Created by 19269 on 2022/7/7.
//
//
#include <unistd.h>
#include <iostream>
#include "lock/lock.h"
#include "pool/threadpool.h"
#include "./epoll/epoller.h"
#include <pthread.h>
#include "./webserver/webserver.h"
#include "./buffer/buffer.h"
#include <unordered_map>
#include <algorithm>
#include "./log/log.h"
//#include <cstdlib>
int p(char* a) {
    return  0;
}
int main(int argc, char *argv[])
{
//    if (argc <= 1)
//    {
//        std::cout << "请按如下方式运行程序: ./" <<"portname\n";
//        exit(-1);
//    }
    // 获取端口号
    int port = atoi(argv[1]); // atoi: convert string to int
    // 通过终端输入定时器定时时间
    int timeout = -1;
    int triMode = 1; // lt(listenFd) & et(connectedFd)
    int threadNum = 8;
    int logLevel = 3;
//    if(argc > 5){ logLevel = atoi(argv[5]); }
//    else if(argc > 4){ threadNum = atoi(argv[4]); }
//    else if(argc > 3){ triMode = atoi(argv[3]); }
//    else if(argc > 2){ timeout = atoi(argv[2]); }
//
    Webserver server(port, triMode, threadNum, logLevel, timeout); // 一般设置 60s 的定时 ， why？
    server.start();

    //test tmie
    // current date/time based on current system
//    time_t now = time(0);
//
//    std::cout << "Number of sec since January 1,1970 is:: " << now << std::endl;
//
//    tm *ltm = localtime(&now);
//
//    // print various components of tm structure.
//    std::cout << "Year:" << 1900 + ltm->tm_year<<std::endl;
//    std::cout << "Month: "<< 1 + ltm->tm_mon<< std::endl;
//    std::cout << "Day: "<< ltm->tm_mday << std::endl;
//    std::cout << "Time: "<< ltm->tm_hour << ":";
//    std::cout << ltm->tm_min << ":";
//    std::cout << ltm->tm_sec << std::endl;
//    std::queue<std::string> _bufferA;
//    _bufferA.push("sasd");
//    _bufferA.push("sasd");
//    _bufferA.push("sasd");
//    printf("bufferA size = %d",_bufferA.size());
    return 0;
}
