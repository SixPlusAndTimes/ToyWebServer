//
// Created by 19269 on 2022/7/13.
//

#include "buffer.h"

Buffer::Buffer(int bufsize)
{
    bufStartPtr = new char[bufsize];
    bufEndPtr = bufStartPtr + bufsize;
    m_readPos = 0;
    m_writePos = 0;
}
Buffer::~Buffer()
{
    _free();
}

void Buffer::_init()
{
    bzero(bufStartPtr, getBufSize());
    m_readPos = 0;
    m_writePos = 0;
}

//返回buffer的字节大小
size_t Buffer::getBufSize() const {
    return bufEndPtr - bufStartPtr;//指针减法
}

void Buffer::_free()
{
    delete[] bufStartPtr;
}

/*重载三个append函数,实现三种参数的传入 :
 *  char*  // 其他两个函数都是调用这个函数
 *  string
 *  void*
 * */
void Buffer::append(const char *str, size_t len) {
    //printf("function:Buffer::append(const char *str, size_t len)\n");
    if (str == nullptr)
    {
        printf("error:invalid str\n");
        return;
    }
    //确保缓冲区能够存放这么多的数据,否则由ensureWtriteble函数扩容
    ensureWriteable(len);
    // 移动元素
    for (size_t i = 0; i < len; ++i)
    {
        bufStartPtr[m_writePos + i] = str[i];
    }
    m_writePos += len; // 更新写指针的位置
}

void Buffer::append(const std::string &str) {
    //string::data() -> https://cplusplus.com/reference/string/string/data/
    append(str.data(), str.length());
}

//len的单位是字节?
void Buffer::append(const void *data, size_t len) {
    if (data == nullptr)
    {
//        LOG_ERROR("error:invalid str");
        printf("error:invalid str\n");
        return;
    }
    append((const char*) data, len);
}

//返回缓冲区内可写字节数
inline
size_t Buffer::writeableBytes() const {
    return getBufSize() - m_writePos;
}

//返回缓冲区内可读取字节数

size_t Buffer::readableBytes() const {
    return m_writePos - m_readPos;
}

//确保缓冲区还有 len 个字节可写
void Buffer::ensureWriteable(size_t len) {
    // buffer的剩余可写空间
    size_t leftSapce = getBufSize() - m_writePos;
    if(leftSapce < len) {
        std::cout << "缓冲区扩容\n";
        allocateSpace(len);
    }
    leftSapce = getBufSize() - m_writePos;
    if(leftSapce < len) {
        std::cout<<"扩容失败\n";
    }
}

void Buffer::allocateSpace(size_t len) {
    //将缓冲区的大小改为 原长度 + 已写长度  的两倍
    size_t newlen = 2 * (len + m_writePos);
    char* newBufArray = new char[newlen];
    for(int i = 0; i < m_writePos; i ++) {
        newBufArray[i] = bufStartPtr[i];
    }
    //删除原来缓冲区的内容
    delete[] bufStartPtr;
    //bufStartPtr指向新的数据区起始地址
    bufStartPtr = newBufArray;
    bufEndPtr = newBufArray + newlen;
}

//缓存区中已经读取的字节数
inline
size_t Buffer::haveReadBytes() const
{
    return m_readPos;
}
//缓冲区中已经写入的字节数
inline
size_t Buffer::haveWriteBytes() const
{
    return m_writePos;
}

void Buffer::updateReadPtr(size_t pos)
{
    m_readPos += pos;
}


std::string Buffer::_all2str()
{
    m_str.assign(bufStartPtr, haveWriteBytes());
    _init();
    return m_str; // 为什么这里可以返回一个局部变量？
}

const char *Buffer::beginPtr() const {
    return  bufStartPtr;
}

// 从sockfd中读取数据到缓冲区
// 返回值：读取正确，返回读取到的字节数；读取错误，对端断线，返回-1
ssize_t Buffer::recvFd(int sockfd, int *Errno)
{
    // 使用readv进行读取
    //cat /proc/sys/net/ipv4/tcp_rmem 查看tcp缓冲区,中间值为默认大小. aliyun服务器为131072 ,所以65536是否太小?
    char extraBuf[65536] = {0}; // 额外扩展空间 , 64KB
    // buffer的剩余写空闲
    size_t writeable = writeableBytes();
    // 分散读写的结构体
    struct iovec iov[2];
    // 装填结构体
    iov[0].iov_base = const_cast<char *>(curWritePtr());
    iov[0].iov_len = writeable;
    iov[1].iov_base = extraBuf;
    iov[1].iov_len = sizeof(extraBuf);
    // 调用readv
    ssize_t len = readv(sockfd, iov, 2);
    printf("readv...\n");
    if(len < 0)
    {
        // 错误发生,设施错误号
        Errno = &errno;
    }
    else if(len <= writeable)
    {
        // 没有用到额外的临时缓冲区
        // 直接将写指针加len
        m_writePos += len;
    }
    else
    {
        //将写指针移动到最后
        m_writePos = getBufSize();
        //调用append将临时缓冲区拷贝到buffer类中的缓冲区
        //在函数内部会将缓冲区扩容
        append(extraBuf, len - writeable);
    }
    return len;
}

ssize_t Buffer::writeFd(int sockfd, int *Errno)
{
    // 调用send函数，向sockfd中发送数据
    ssize_t len = 0;
    ssize_t sendBytes = 0;
    while(true)
    {
        len = send(sockfd, curReadPtr(), readableBytes() - len, 0);
        if (len == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                //被信号打断,重发
                *Errno = errno;
                break;
            }
            *Errno = errno;
            return -1;
        }
        if (len == 0)
        {
            // 对端关闭了连接
            *Errno = errno;
            return -1;
        }
        m_readPos += len;
        sendBytes += len;
    }
    return sendBytes;
}
