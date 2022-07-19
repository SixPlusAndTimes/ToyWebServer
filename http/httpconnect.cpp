//
// Created by 19269 on 2022/7/14.
//

#include "httpconnect.h"
/*初始化类static变量*/
bool Httpconnection::isET = false;
// 在WebServer.cpp中会被赋值修改路径
const char *Httpconnection::srcDir = "";
std::atomic<int> Httpconnection::userCount(0);

Httpconnection::Httpconnection()
{
    m_fd = -1;
    m_addr = {0};
    isClose = true;
    isGetRightResource = false;
    isHttpDone = false;
}
Httpconnection::~Httpconnection()
{
    closeHTTPConn();
}

//关闭HTTP连接的接口,调用close(fd)关闭tcp连接,并将httpconnection::userCount--
void Httpconnection::closeHTTPConn()
{
//    m_response._unmapFile();
    if(isClose == false)
    {
        isClose = true;
        userCount--;
        close(m_fd);
        m_fd = -1;
//        LOG_INFO("[%d]_%s:%d[OUT], usrCnt[%d]",
//                 getFd(), getIP(), getPort(), (int)Httpconnection::userCount);
    }
}
void Httpconnection::initHTTPConn(int socketFd, const sockaddr_in &addr)
{
    if(socketFd < 0) {
        std::cout << "invalid socket fd";
    }
    userCount++;
    m_addr = addr;
    m_fd = socketFd;
    httpReadBuf._init();
    httpWriteBuf._init();
    isClose = false; // 连接中
    printf("http Conected , IP : %s  , PORT : %d",getIP(),getPort() );
}

//将示例对应的fd缓冲区的内容读到自己的m_readBuf中
ssize_t Httpconnection::readBuffer(int *saveErrno) {
    printf("Httpconnection::readBuffer() begin...\n");
    ssize_t len = 0;
    ssize_t sum = 0;
    do {
        len = httpReadBuf.recvFd(m_fd,saveErrno);
        if(len <= 0) break;
        sum += len;
    }while(isET);
    return sum;
}

//解析http请求，并向对端发送响应
bool Httpconnection::handleHTTPConn() {
    m_request._init();
    if(httpReadBuf.readableBytes() <= 0)
    {
        return false;
    }
    bool ret = m_request.parse(httpReadBuf);
    if(ret) {

    }
    return true;
}

const char *Httpconnection::getIP() const
{
    return inet_ntoa(m_addr.sin_addr);
}
int Httpconnection::getPort() const
{
    return ntohs(m_addr.sin_port);
}

int Httpconnection::getFd() const
{
    return m_fd;
}

sockaddr_in Httpconnection::getAddr() const
{
    return m_addr;
}
inline
bool Httpconnection::isKeepAlive() const {
    return true;
}

inline
int Httpconnection::writeBytes() {
    return m_iov[0].iov_len + m_iov[1].iov_len;
}