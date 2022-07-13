//
// Created by 19269 on 2022/7/12.
//

#ifndef TOYWEBVSERVER_WEBSERVER_H
#define TOYWEBVSERVER_WEBSERVER_H
#include "../epoll/epoller.h"
#include <fcntl.h>       // fcntl()
#include <unistd.h>      // close()
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory>
class Webserver
{
public:
    Webserver(int port, int triMode, int threadNum, int LogLevel, int timeOut);

    ~Webserver();

    void start();//启动服务器

private:
    bool initSocket();
    void initEventMode(int trigMode);
    void addClientConnect(int fd, struct sockaddr_in addr); // 添加一个连接
//    void closeConn(Httpconnection *client);
//    void delClient(Httpconnection *client);


    void handleListen();
//    bool handleWrite(Httpconnection *client);
//    bool handleRead(Httpconnection *client);

    static const int MAX_FD = 65535;
    static int setNONBLOCKING(int fd);

    int m_port;
    int m_timeoutMs;
    int m_listenFd;
    bool m_isclose;//服务器关闭标志

    uint32_t m_listenFdEventFlag;//listen()中fd的epoll flag
    uint32_t m_connectFdEventFlag; //accept后的fd 的epoll flag

    std::unique_ptr<Epoller> m_epoller;
};


#endif //TOYWEBVSERVER_WEBSERVER_H
