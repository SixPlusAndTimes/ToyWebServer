//
// Created by 19269 on 2022/7/25.
//

#include <iostream>
#include "config.h"
Config::Config() {
    //端口号,默认9006
    port = 10000 ;

    //触发组合模式,默认listenfd LT + connfd LT
    trig_mode = 1;

    //数据库连接池数量,默认8
    sql_num = 8;

    //线程池内的线程数量,默认8
    thread_num = 8;

    //日志等级默认 1 小于DEBUG要求的等级
    log_level = 1;

    time_out = -1;
}
void Config::parseArgs(int argc, char **argv) {
    int opt;
    //p 端口
    //m trigmode
    //s sql thread nums
    //t server thread nums
    //g log level
    const char *str = "p:m:s:t:l:o:";
    while ((opt = getopt(argc, argv, str)) != -1)
    {
        switch (opt)
        {
            case 'p' :
            {
//                std::cout << "-p\n";
                port = atoi(optarg);
                break;
            }
            case 'm':
            {
//                std::cout << "-m\n";
                trig_mode = atoi(optarg);
                break;
            }
            case 's':
            {
//                std::cout << "-s\n";
                sql_num = atoi(optarg);
                break;
            }
            case 't':
            {
//                std::cout << "-t\n";
                thread_num = atoi(optarg);
                break;
            }
            case 'l' :
            {
//                std::cout << "-l\n";
                log_level = atoi(optarg);
//                std::cout <<"log_level : " << log_level <<std::endl;
                break;
            }
            case 'o':
            {
//                std::cout << "-o\n";
//                std::cout << optarg;
                time_out = atoi(optarg);
//                std::cout <<"timeout : " << time_out <<std::endl;
                break;
            }
            default:
                break;
        }
    }
}