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
//#include <cstdlib>
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
    int triMode = 1;
    int threadNum = 8;
    int logLevel = 1;
//    if(argc > 5){ logLevel = atoi(argv[5]); }
//    else if(argc > 4){ threadNum = atoi(argv[4]); }
//    else if(argc > 3){ triMode = atoi(argv[3]); }
//    else if(argc > 2){ timeout = atoi(argv[2]); }
//
    Webserver server(port, triMode, threadNum, logLevel, timeout); // 一般设置 60s 的定时 ， why？
    server.start();

//    std::unordered_map<int,Buffer> map;
//    auto b =map[0];
//    b._init();
//    b.append("asd");
//    std::cout << b._all2str();
//    while(1);

//    std::cout << "tes";
//    Buffer b;
//    b.append("asdj");
//    b.append("asaca");
//    b.append(std::string("aspo"));
//    std::cout << b._all2str() << std::endl;
    return 0;
}
