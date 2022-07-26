//
// Created by 19269 on 2022/7/12.
//


#include "webserver.h"

#include<iostream>

Webserver::Webserver(int port, int triMode, int threadNum, int logLevel, int timeOut)
        :m_port(port), m_timeoutMs(timeOut), m_timer_manager(new TimerManager()),m_isclose(false), m_epoller(new Epoller()), m_threadpool(new threadpool(8,100))
{
    getcwd(m_srcDir,sizeof(m_srcDir));
    char* lastSlash =  strrchr(m_srcDir,'/');
    *lastSlash = '\0';
    strcat(m_srcDir,"/resources");
    std::cout << "webserver's dir = " << m_srcDir <<std::endl;
    Httpconnection::srcDir = m_srcDir;
    Httpconnection::userCount = 0;

    initEventMode(triMode);//初始化触发模式
    m_isclose = initSocket() == true ? false : true;

    Log::getInstance()->setLevel(logLevel);

    LOG_DEBUG("logLeve = %d",logLevel);
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
    Httpconnection::isET = m_connectFdEventFlag & EPOLLET;
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
       printf("error epoll_add\n");
        close(m_listenFd);
        return false;
    }
    return true;
}

void Webserver::start()
{
    if (!m_isclose)
    {
//        std::cout << "============================\n";
//        std::cout << "Server Start!\n";
//        std::cout << "============================\n";
//        std::cout << std::endl;

        LOG_INFO("\n==== Server Start ====\n\
                    listenEvent: %s, connectEvent: %s\n\
                    timer set: %d ms, log level: %s\n\
                    resources: [%s]",
                 m_listenFdEventFlag & EPOLLET ? "ET" : "LT",
                 m_connectFdEventFlag & EPOLLET ? "ET" : "LT",
                 m_timeoutMs, Log::getInstance()->getLevel().c_str(), m_srcDir);

        printf("\n==== Server Start ====\n\
                    listenEvent: %s, connectEvent: %s\n\
                    timer set: %d ms, log level: %s\n\
                    resources: [%s]",
                 m_listenFdEventFlag & EPOLLET ? "ET" : "LT",
                 m_connectFdEventFlag & EPOLLET ? "ET" : "LT",
                 m_timeoutMs, Log::getInstance()->getLevel().c_str(), m_srcDir);
    }
    while (!m_isclose)
    {
        int waitTime = -1;
        if (m_timeoutMs > 0)
        {
            //第一次为 -1
            waitTime = m_timer_manager->getNextHandle();
            LOG_DEBUG("waitTime = %d",waitTime);
//            std::cout << waitTime <<std::endl;
        }
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
                LOG_DEBUG("NewConnectionComming");
//                std::cout << "New connection comming\n";
                handleListen(); // 处理新连接
            }
            else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                // 对端关闭了连接
                closeConn(&m_usrs[currfd]);
                std::cout << "Opposite End Socket Close!!\n";
            }
            else if (events & EPOLLIN)
            {
//                std::cout << "======================readEvent!=====================\n";
                // 读事件  // 现在主线程中读取数据到缓冲区中；
                // 再由线程池完成业务逻辑 // 读取http请求
                handleRead(&m_usrs[currfd]);

            }
            else if (events & EPOLLOUT)
            {
                // 写事件
//                std::cout << "======================writeEvent!=====================\n";
                handleWrite(&m_usrs[currfd]);
//                printf("handle return \n");

            }
            else
            {
                std::cout << "something unknown event!\n";

            }
        }
    }
}
/*调用accept 产生connectFD, 判断http连接是否超出限制；
 * 调用addClientConnect将fd加入epoller中
 * */
void Webserver::handleListen()
{
    struct sockaddr_in caddr;
    socklen_t len = sizeof(caddr);
    do
    {
//        struct timeval t1;
//        gettimeofday(&t1, NULL);
//        std::cout << "handling listen...\n";
        int cfd = accept(m_listenFd, (struct sockaddr *)&caddr, &len);
        if (cfd < 0)
        {
            std::cout << "accept failed!";
            return;
        }
            // 如果客户端数量超过上限
        else if (Httpconnection::userCount >= MAX_FD)
        {
            // 调用send函数向客户端发送错误信息
            // 更加严谨的应该是返回一个html网页
//            sendError(cfd, "Server Busy");
            printf("Httpconnection::userCount >= MAX_FD)!!\n");
            return;
        }
//          调用封装函数，将新来的客户端的cfd和caddr加入到服务器中
        addClientConnect(cfd, caddr);

        m_epoller->add(cfd, m_connectFdEventFlag | EPOLLIN);
    } while (m_listenFdEventFlag & EPOLLET); //如果listenfd是ET模式，应该循环读fd
}

/*在初始化一个HTTP连接，然后将这个连接放入map中，将fd加入epoller中监听
 * 并且为fd设置非阻塞
 * */
void Webserver::addClientConnect(int fd, struct sockaddr_in addr) {
    m_usrs[fd].initHTTPConn(fd, addr);
    m_epoller->add(fd,m_connectFdEventFlag | EPOLLIN);

    //超时逻辑处理
    if (m_timeoutMs > 0)
    {
        m_timer_manager->addTimer(fd, m_timeoutMs, std::bind(&Webserver::delClient, this, &m_usrs[fd]));
    }
    //将fd设置为非阻塞
    setNONBLOCKING(fd);
}



/*由主线程读取数据，将业务处理逻辑交给线程池中的线程处理
 * 模拟proactor模式
 * 主线程调用http::readBuffer将，fd缓冲区的内容读到，httpconnect中的readBuffer中
 * */
bool Webserver::handleRead(Httpconnection *client) {
    if(client == nullptr) {
        std::cout << "handleRead() error";
        return false;
    }
    // 在主线程中，读取数据
    int errorNum = 0;
    struct timeval t1;
    gettimeofday(&t1, NULL);
    int ret = client->readBuffer(&errorNum);
    if (ret <= 0 && errorNum != EAGAIN)
    {
        // 读取数据出现错误，关闭客户端
        printf("handleRead: 读取数据错误，errno:%d\n", errorNum);
        closeConn(client);
        return false;
    }
    // 更新一下定时器
    if (m_timeoutMs > 0)
    {
        m_timer_manager->updateTimer(client->getFd(), m_timeoutMs);
    }
    struct timeval t2;
    gettimeofday(&t2, NULL);
    long tt = (t2.tv_sec-t1.tv_sec)*1000000 + (t2.tv_usec-t1.tv_usec);
//    printf("clientfd = %d, 数据读取完成，耗时： %d\n", client->getFd(), tt);
    // 把业务处理逻辑代码交给线程池去处理

    m_threadpool->append(std::bind(&Webserver::onRead, this, client));
    return  true;
}


void Webserver::onRead(Httpconnection *client) {
    onProcess(client);
}
//处理一个HTTP请求,所需的数据已经在httpReadBuffer中
void Webserver::onProcess(Httpconnection *client) {
//    printf("WebServer::onProcess()\n");
    if (client->handleHTTPConn())
    {
        //处理http请求成功，将fd的事件改为写事件；此时httpwriteBuff也已经填充了要发给客户端的数据
        client->clearHttpReadBuffer();//清空httpreadbuffer
        m_epoller->mod(client->getFd(), m_connectFdEventFlag | EPOLLOUT);
//        std::cout << "================handleHttoConn succeed!now modify fd to epoll event=========" << std::endl;
    }
    else
    {
        //http请求不完整？ 继续监听读事件
        m_epoller->mod(client->getFd(), m_connectFdEventFlag | EPOLLIN);
    }
}

/*
 * 首先调用epoller的del方法，将它从监听列表删除
 * 然后调用httpconnetction中的closeHTTPConn方法，从而close(fd)
 * */
void Webserver::closeConn(Httpconnection *client)
{

    if (client == nullptr)
    {
//        LOG_ERROR("invalid client ptr");
        return;
    }
    if (!m_epoller->del(client->getFd()))
    {
        printf("%s:%d epoll_del failed\n", client->getIP(), client->getPort());
    }
    if (m_timeoutMs > 0)
    {
        m_timer_manager->delFd(client->getFd());
    }
    client->closeHTTPConn();
}

int Webserver::setNONBLOCKING(int fd) {
    int oldflag = fcntl(fd, F_GETFL);
    int newflag = oldflag | O_NONBLOCK;
    fcntl(fd, F_SETFL, newflag);
    return newflag;
}

// 提供给定时器管理类的删除客户端函数
void Webserver::delClient(Httpconnection *client)
{
    if (client == nullptr)
    {
//        LOG_ERROR("invalid client ptr");
        return;
    }
    if (!m_epoller->del(client->getFd()))
    {
//        LOG_DEBUG("%s:%d epoll_del failed", client->getIP(), client->getPort());
    }
    client->closeHTTPConn();
//    int cnt = Httpconnection::userCount;
    // LOG_INFO("[%d]-%s:%d[OUT], usrCnt[%d]",
    //          client->getFd(), client->getIP(), client->getPort(), cnt);
}

/*调用httpClient::writeBuffer()函数
 * 如果调用失败，根据不同的状况执行不同的善后操作
 * */
bool Webserver::handleWrite(Httpconnection *client) {
    if(client == nullptr){
        return false;
    }
    int erroNo = 0;
    ssize_t ret = client->writeBuffer(&erroNo);
    // 更新定时器
    if (m_timeoutMs > 0)
    {
        m_timer_manager->updateTimer(client->getFd(), m_timeoutMs);
    }
//    printf(" in Webserver::handleWrite -- hclient->writeBuffer() return \n");
    bool statusRecord = false;
    if (ret > 0)
    {

        if (client->isKeepAlive())
        {
            statusRecord = true;
            m_epoller->mod(client->getFd(), m_connectFdEventFlag | EPOLLIN);
//            LOG_DEBUG("数据发送完成");
        }
        else
        {
//            printf("[%d]数据发送长度大于0，但是客户端选择不保持连接\n",client->getFd());
            closeConn(client);
            statusRecord = false;
        }
    }
    else if (client->writeBytes() == 0)
    {
        // 保存的客户端属性中，此时没有数据
        if (client->isKeepAlive())
//        if(true)
        {
            // 对于长连接，如果没有数据，那就重新更新一个客户端中数据
//            printf("发送的数据长度为0，重新进入到onProcess阶段\n");
            onProcess(client);
            statusRecord = true;
        }
        statusRecord = false;
    }
    else if (ret < 0)
    {
        if (erroNo == EAGAIN)
        {
            // LOG_DEBUG("发送的数据长度小于0，但是错误号为EAGAIN");
            m_epoller->mod(client->getFd(), m_connectFdEventFlag | EPOLLOUT);
            statusRecord = true;
        }
        statusRecord = false;
    }
    return statusRecord;
}