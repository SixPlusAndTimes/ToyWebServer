//
// Created by 19269 on 2022/7/25.
//

#ifndef TOYWEBVSERVER_CONFIG_H
#define TOYWEBVSERVER_CONFIG_H
#include <unistd.h>
#include <cstdlib>

class Config {
public:
    Config();
    ~Config(){};

    void parseArgs(int argc, char*argv[]);

    //端口号
    int port;

    //触发组合模式
    int trig_mode;


    //数据库连接池数量
    int sql_num;

    //线程池内的线程数量
    int thread_num;

    //日志等级
    int log_level;

    //超时时间
    int time_out;

};


#endif //TOYWEBVSERVER_CONFIG_H
