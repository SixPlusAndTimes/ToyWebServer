//
// Created by 19269 on 2022/7/13.
//

#ifndef TOYWEBVSERVER_BUFFER_H
#define TOYWEBVSERVER_BUFFER_H
#include <iostream>
#include <string>
//#include <alloca.h>
#include <unistd.h>
#include <assert.h>
#include <strings.h> /* bzero */
#include <sys/types.h> /*ssize_t, 与 size_t相比,它是有符号的*/
#include <sys/socket.h> /* recv */
#include <sys/uio.h> /* readv */

//#define BUF_SIZE_INIT 128
// 《effective C++》 item 2：尽量以const、enum、inline替换 #define
const int DEFAULT_BUF_SIZE_INIT = 128; //buffer的初始大小

class Buffer
{
public:
    Buffer(int bufsize = DEFAULT_BUF_SIZE_INIT);
    ~Buffer();
    // 向缓冲区写入数据
    void append(const char *str, size_t len);
    void append(const std::string &str);
    void append(const void *data, size_t len);
    // void append(const Buffer &buf);
    // 将缓冲区中的数据，发送到socket通信的对端
    ssize_t writeFd(int sockfd, int *Errno);
    // 从socket通信的对端，接收数据到缓冲区
    ssize_t recvFd(int sockfd, int *Errno);
    //缓冲区的大小
    size_t getBufSize() const;
    //缓存区中可以读取的字节数
    size_t writeableBytes() const;
    //缓存区中可以写入的字节数
    size_t readableBytes() const;
    //缓存区中已经读取的字节数
    size_t haveReadBytes() const;
    //缓冲区中已经写入的字节数
    size_t haveWriteBytes() const;
    // 缓冲区头。有了缓冲区头+写入的数据长度，便可以在类外使用writev函数进行写入
    const char *beginPtr() const;
    // 当前的读取指针的位置
    const char *curReadPtr() const {return bufStartPtr + m_readPos;}
    // 当前的写入指针的位置
    const char *curWritePtr() const {return bufStartPtr + m_writePos;}
    // 更新读指针的位置：读指针是对外使用的接口，外部调用读取数据，要及时更新指针的位置
    void updateReadPtr(size_t pos);
    // 将缓冲区中的内容输出成string
    std::string _all2str();
    // 将指针和变量初始化
    void _init();

private:
    // 检查空间是否足够
    void ensureWriteable(size_t len);
    // 扩容 : 获得更多内存并拷贝已有元素
    void allocateSpace(size_t len);
    // 销毁元素并释放内存
    void _free();

    // 底层的数据结构，char数组
    // char数组的头指针
    char *bufStartPtr;
    // char数组的尾指针
    char *bufEndPtr;
    // string
    std::string m_str;

    size_t m_readPos;
    size_t m_writePos;
};
#endif //TOYWEBVSERVER_BUFFER_H
