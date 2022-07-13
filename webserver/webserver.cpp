//
// Created by 19269 on 2022/7/12.
//


#include "webserver.h"
#include<iostream>

Webserver::Webserver(int port, int triMode, int threadNum, int LogLevel, int timeOut)
        :m_port(port), m_timeoutMs(timeOut), m_isclose(false), m_epoller(new Epoller())
{

    initEventMode(triMode);//初始化触发模式
    initSocket();
}
Webserver::~Webserver(){};
// 对服务器程序进行初始化的函数
/*注意 ： epoll事件默认为LT
 * trigMode = 1 ：  只有m_connectFdEventFlag为ET
 * triMode = 2 ： 只有m_listenFdEventFlag为ET
 * triMode = 3 : 两种都是ET
 * triMode = 其他 ： 都默认为LT
 * */
void Webserver::initEventMode(int trigMode)
{
    // EPOLLRDHUP：当对端关闭socket时触发的事件
    m_listenFdEventFlag = EPOLLRDHUP;
    /* EPOLLONESHOT 和 ET模式不尽相同：
    前者是防止一个客户端发送的数据被多个线程分散读取；
    后者是避免多次调用epoll_wait提高epoll效率的一种模式
    */
    m_connectFdEventFlag = EPOLLRDHUP | EPOLLONESHOT;
    switch (trigMode)
    {
        case 1:
            m_connectFdEventFlag |= EPOLLET;
            break;
        case 2:
            m_listenFdEventFlag |= EPOLLET;
            break;
        case 3:
            m_connectFdEventFlag |= EPOLLET;
            m_listenFdEventFlag |= EPOLLET;
            break;
        default:
            // 其他就是都LT
            break;
    }
//    Httpconnection::isET = m_connectEvent & EPOLLET;
}

/*
 * 1. 创建listen_fd
 * 2.
 * */
bool Webserver::initSocket()
{
    m_listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listenFd < 0)
    {
        return false;
    }
    // bind
    struct sockaddr_in saddr;
    //初始化结构体addr
    saddr.sin_family = AF_INET;
    // inet_pton(AF_INET, "10.0.12.8", (void *)&saddr.sin_addr.s_addr);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(m_port);
    int reuse = 1;
    int ret = setsockopt(m_listenFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    if (ret < 0)
    {
//        LOG_ERROR("error setsockopt");
        close(m_listenFd);
        return false;
    }

    ret = bind(m_listenFd, (struct sockaddr *)&saddr, 16);
    if (ret < 0)
    {
//        LOG_ERROR("error bind");
        close(m_listenFd);
        return false;
    }
    // listen(fd, backlogNum)
    /* Man： about backlogNum
     * The behavior of the backlog argument on TCP sockets changed with Linux 2.2.
     * Now it specifies the queue length for completely  established  sockets  waiting to be accepted,
     * instead of the number of incomplete connection requests.*/
    ret = listen(m_listenFd, 100);
    if (ret < 0)
    {
//        LOG_ERROR("error listen")
        close(m_listenFd);
        return false;
    }
    // 将m_listenFd添加到epoll中
    ret = m_epoller->add(m_listenFd, m_listenFdEventFlag | EPOLLIN);
    if (ret == 0)
    {
//        LOG_ERROR("error epoll_add");
        close(m_listenFd);
        return false;
    }
    return true;
}

void Webserver::start()
{
    if (!m_isclose)
    {
        std::cout << "============================";
        std::cout << "Server Start!";
        std::cout << "============================";
        std::cout << std::endl;
    }
    while (!m_isclose)
    {
        int waitTime = -1;
//        if (m_timeoutMs > 0)
//        {
//            waitTime = m_timer->getNextHandle();
//        }
        // 调用epoll监听
        int eventCnt = m_epoller->wait(waitTime);
        for (int i = 0; i < eventCnt; ++i)
        {
            // 获取fd
            int currfd = m_epoller->getSocktFd(i);
            // 获取fd对应的event
            uint32_t events = m_epoller->getFdEvent(i);

            // 判断事件类型：新连接到来？读？写？
            if (currfd == m_listenFd)
            {
                handleListen(); // 处理新连接
            }
            else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                // 对端关闭了连接
//                closeConn(&m_usrs[currfd]);
                std::cout << "Opposite End Socket Close!!\n";
            }
            else if (events & EPOLLIN)
            {
                // 读事件  // 现在主线程中读取数据到缓冲区中；
                // 再由线程池完成业务逻辑 // 读取http请求
//                handleRead(&m_usrs[currfd]);
                std::cout << "readEvent!\n";
            }
            else if (events & EPOLLOUT)
            {
                // 写事件
//                handleWrite(&m_usrs[currfd]);
                std::cout << "writeEvent!\n";
            }
            else
            {
                std::cout << "something unknown event!\n";

            }
        }
    }
}

void Webserver::handleListen()
{
    struct sockaddr_in caddr;
    socklen_t len = sizeof(caddr);
    do
    {
//        struct timeval t1;
//        gettimeofday(&t1, NULL);
        std::cout << "handling listen...\n";
        int cfd = accept(m_listenFd, (struct sockaddr *)&caddr, &len);
        if (cfd < 0)
        {
            std::cout << "accept failed!";
            return;
        }
            // 如果客户端数量超过上限
//        else if (Httpconnection::userCount >= MAX_FD)
//        {
//            // 调用send函数向客户端发送错误信息
//            // 更加严谨的应该是返回一个html网页
//            sendError(cfd, "Server Busy");
//            return;
//        }
        //  调用封装函数，将新来的客户端的cfd和caddr加入到服务器中
//        addClientConnect(cfd, caddr);

        m_epoller->add(cfd, m_connectFdEventFlag | EPOLLIN);
    } while (m_listenFdEventFlag & EPOLLET); //如果listenfd是ET模式，应该循环读fd
}