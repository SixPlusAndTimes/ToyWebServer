//
// Created by 19269 on 2022/7/25.
//

#include <iostream>
#include "config.h"
Config::Config() {
    //端口号,默认9006
    port = 9006;

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
    const char *str = "p:m:s:t:l:";
    while ((opt = getopt(argc, argv, str)) != -1)
    {
        switch (opt)
        {
            case 'p' :
            {
                port = atoi(optarg);
                break;
            }
            case 'm':
            {
                trig_mode = atoi(optarg);
                break;
            }
            case 's':
            {
                sql_num = atoi(optarg);
                break;
            }
            case 't':
            {
                thread_num = atoi(optarg);
                break;
            }
            case 'l' :
            {
                log_level = atoi(optarg);
                break;
            }
            case 'o':
            {
                time_out = atoi(optarg);
            }
            default:
                break;
        }
    }
}