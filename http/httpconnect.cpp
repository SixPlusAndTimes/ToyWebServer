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
//        printf("in httpconnect::closeHttpConn  close client fd\n");
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
//    printf("http Conected , IP : %s  , PORT : %d",getIP(),getPort() );
}

//将示例对应的fd缓冲区的内容读到自己的m_readBuf中
ssize_t Httpconnection::readBuffer(int *saveErrno) {
//    printf("Httpconnection::readBuffer() begin...\n");
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
        std::cout << "parse request succeed . m_request::path = " << m_request.path() <<std::endl;
        m_response.init(srcDir,m_request.path(),m_request.isKeepAlive(),200);
    } else{
        //http 解析不成功
        std::cout<<"in handleHTTPConn() :请求错误\n";
        m_response.init(srcDir,m_request.path(),false,400);
    }
    ret = m_response.makeResponse(httpWriteBuf);

    m_iov[0].iov_base = const_cast<char *>(httpWriteBuf.curReadPtr());
    m_iov[0].iov_len = httpWriteBuf.readableBytes();

    std::cout << "m_iov[0].iov_len = " << httpWriteBuf.readableBytes() << std::endl;
    std::cout << "httpWriteBuf.curWritePtr = " << httpWriteBuf.curReadPtr() << std::endl;
    if(m_response.fileLen() > 0 && m_response.file() )
    {
        //如果有文件要发送，则将文件装填在iov的第二个元素中
        std::cout << "in handleHTTPConn() : 正在装填m_iov分散写结构体\n";
        m_iov[1].iov_base = m_response.file();
        m_iov[1].iov_len = m_response.fileLen();
        m_iovCnt = 2;
    }
    return true;
}

//将httpWriteBUffer中的数据通过socket发送给对方
ssize_t Httpconnection::writeBuffer(int *saveErrno) {
    printf("Httpconnection::writeBuffer(): [%d]_开始发送资源\n", m_fd);
    ssize_t len = -1;
    ssize_t _writebytes = 0;
    do{
        len = writev(m_fd, m_iov, m_iovCnt);
//        printf("send %d bytes\n",len);
        if(len < 0) {
            *saveErrno = errno;
            return -1;
        }
        if(m_iov[0].iov_len + m_iov[1].iov_len == 0)
        {
            std::cout << "transmiting done !!\n";
            break;
        }
        else if(static_cast<size_t>(len) > m_iov->iov_len)
        {
            m_iov[1].iov_base = (char *)m_iov[1].iov_base + (len - m_iov[0].iov_len);
            m_iov[1].iov_len -= (len - m_iov[0].iov_len);
            if(m_iov[0].iov_len)
            {
                httpWriteBuf._init();
                m_iov[0].iov_len = 0;
            }
        }else {
            m_iov[0].iov_base = (char *)m_iov[0].iov_base + len;
            m_iov[0].iov_len -= len;
            httpWriteBuf.updateReadPtr(static_cast<size_t>(len));
        }
        _writebytes += len;
    }while(isET || writeBytes() > 10240); // writeBytes() > 10240 这是啥？？
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

bool Httpconnection::isKeepAlive() const {
    return m_request.isKeepAlive();
}


int Httpconnection::writeBytes() {
    return m_iov[0].iov_len + m_iov[1].iov_len;
}

void Httpconnection::clearHttpReadBuffer() {
    httpReadBuf._init();
}