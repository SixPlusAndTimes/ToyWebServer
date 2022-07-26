//
// Created by 19269 on 2022/7/7.
//
//

#include "lock/lock.h"
#include "pool/threadpool.h"
#include "./epoll/epoller.h"
#include "./webserver/webserver.h"
#include "./buffer/buffer.h"
#include <algorithm>
#include "./config/config.h"
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

    Config config;
    config.parseArgs(argc, argv);

//    int port = atoi(argv[1]); // atoi: convert string to int
//    // 通过终端输入定时器定时时间
//    int timeout = -1;
//    int triMode = 1; // lt(listenFd) & et(connectedFd)
//    int threadNum = 8;
//    int logLevel = 3;
//    if(argc > 5){ logLevel = atoi(argv[5]); }
//    else if(argc > 4){ threadNum = atoi(argv[4]); }
//    else if(argc > 3){ triMode = atoi(argv[3]); }
//    else if(argc > 2){ timeout = atoi(argv[2]); }
//

    Webserver server(config.port, config.trig_mode, config.thread_num, config.log_level, config.time_out);
    server.start();

    return 0;
}

