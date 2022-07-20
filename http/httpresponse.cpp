//
// Created by 19269 on 2022/7/19.
//

#include "httpresponse.h"
const std::unordered_map<std::string, std::string> Httpresponse::SUFFIX_TYPE = {
        {".html", "text/html"},
        {".xml", "text/xml"},
        {".xhtml", "application/xhtml+xml"},
        {".txt", "text/plain"},
        {".rtf", "application/rtf"},
        {".pdf", "application/pdf"},
        {".word", "application/nsword"},
        {".png", "image/png"},
        {".gif", "image/gif"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".au", "audio/basic"},
        {".mpeg", "video/mpeg"},
        {".mpg", "video/mpeg"},
        {".avi", "video/x-msvideo"},
        {".gz", "application/x-gzip"},
        {".tar", "application/x-tar"},
        {".css", "text/css "},
        {".js", "text/javascript "},
};

const std::unordered_map<int, std::string> Httpresponse::CODE_STATUS = {
        {200, "OK"},
        {400, "Bad Request"},
        {403, "Forbidden"},
        {404, "Not Found"},
};

const std::unordered_map<int, std::string> Httpresponse::CODE_PATH_4XX = {
        {400, "/400.html"},
        {403, "/403.html"},
        {404, "/404.html"},
};

Httpresponse::Httpresponse()
{
    m_code = -1;
    m_path = m_srcDir = "";
    m_keepalive = false;
    m_mmfile = nullptr;
    mmFileState = {0};
}
Httpresponse::~Httpresponse()
{
    _unmapFile();
}

void Httpresponse::init(const std::string srcDir, const std::string path, bool isKeepAlive, int code) {
//    std::cout << "=====Httpresponse::init()...\n";
    if(srcDir == "")
    {
        std::cout<<"invalid srcdir!\n";
        return;
    }

    if(m_mmfile != nullptr) {
        _unmapFile();
    }
    m_code = code;
    m_keepalive = isKeepAlive;
    m_path = path;
    m_srcDir = srcDir;
    m_mmfile = nullptr;
    mmFileState = {0};
//    std::cout << "srcdir = " << srcDir << std::endl;
//    std::cout << "m_code = " << m_code << std::endl;
//    std::cout << "m_keepalive  = " << m_keepalive << std::endl;
//    std::cout << "m_path  = " << m_path << std::endl;
//    std::cout << "=====Httpresponse::init() done\n";

}

void Httpresponse::_unmapFile() {
    if (m_mmfile != nullptr)
    {
        munmap(m_mmfile, mmFileState.st_size);
        m_mmfile = nullptr;
    }
}
//组装状态行， 响应头，和 响应内容
bool Httpresponse::makeResponse(Buffer &buffer) {
    if(stat((m_srcDir + m_path).data(),&mmFileState) < 0 || S_ISDIR(mmFileState.st_mode)) {
        //如果找不到请求的文件，或者文件是目录
        std::cout << "Httpresponse::makeResponse() 找不到文件 : \n" << "\t\t " << m_srcDir + m_path ;
        m_code = 404;
    }else if(!(mmFileState.st_mode & S_IROTH)) {
        //403 forbidden， 无权限
        std::cout << "Httpresponse::makeResponse() 无权限！\n";
        m_code = 403;
    }
    //成功的状态码： 200
    if(m_code == -1) m_code = 200;
    errorHTML();// 如果错误码不为200，则将m_path 改为resource中的404界面
    addStateLine(buffer); //在buffer中增加状态行
    addHeader(buffer); //在buffer中增加响应头
    addContent(buffer);//使用mmap将文件内容映射至内存，将文件在内存的位置存储在m_mmfile中，把文件大小存储在mmFileState中
//    std::cout << ">>>>> buffer : \n";
//    std::cout << buffer._all2str() << std::endl; // 注意 buffer._all2str()清空 buffer 的可读数据，debug时要小心
//    std::cout << "buffer readable byte : "<< buffer.readableBytes()<<std::endl;
    return m_code == 200 ? true : false;
}

void Httpresponse::addStateLine(Buffer &buffer)
{
    std::string status;
    // 从哈希表从查找m_code对应的状态字
    if (CODE_STATUS.count(m_code) > 0)
    {
        status = CODE_STATUS.find(m_code)->second;
    }
    else
    {
        m_code = 400; // BAD REQUEST
        status = CODE_STATUS.find(400)->second;
    }
    // 把响应的字段写入到缓冲区中
    buffer.append("HTTP/1.1 " + std::to_string(m_code) + " " + status + "\r\n");
//    std::cout << "afer append stateline line the readablebytes = " << buffer.readableBytes()<<std::endl;
}

void Httpresponse::addHeader(Buffer &buffer)
{
    buffer.append("Connection: ");
    if (m_keepalive)
    {
        buffer.append("keep-alive\r\n");
        buffer.append("keep-alive: max=6, timeout=60\r\n");
    }
    else
    {
        buffer.append("close\r\n");
    }
    // Content-type
    buffer.append("Content-type: " + getFileType() + "\r\n");
    // content-legnth
        //为啥是两个 \r\n ???
        //http格式规定的 ： https://blog.csdn.net/abcnull/article/details/84787954
    buffer.append("Content-length: " + std::to_string(mmFileState.st_size) + "\r\n\r\n");

//    std::cout << "afer append the header readable bytes = " << buffer.readableBytes()<<std::endl;
}

//将（一般是文件内容），加到buffer中
void Httpresponse::addContent(Buffer &buffer)
{
    // LOG_DEBUG("组装响应报文的BODY部分");
    int srcFd = open((m_srcDir + m_path).data(), O_RDONLY);
    if (srcFd == -1)
    {
        errorContent(buffer, "File NotFound!\r\n");
//        LOG_ERROR("error:open file faild");
        return;
    }
    //内存映射
    int *mmret = (int *)mmap(0, mmFileState.st_size, PROT_READ, MAP_PRIVATE, srcFd, 0);
    if (*mmret == -1)
    {
        errorContent(buffer, "File NotFound!\r\n");
//        LOG_ERROR("error:mmap faild");
        return;
    }
    m_mmfile = (char *)mmret;
    close(srcFd);
    // LOG_DEBUG("响应报文的BODY部分组装完毕");
}

std::string Httpresponse::getFileType()
{
    std::string::size_type idx = m_path.find_last_of('.');
    if (idx == std::string::npos)
    {
        // not found
        return "text/plain";
    }
    std::string suffix = m_path.substr(idx); // 从'.'开始的子串
    if (SUFFIX_TYPE.count(suffix) > 0)
    {
        return SUFFIX_TYPE.find(suffix)->second;
    }
    return "text/plain";
}
char *Httpresponse::file() const
{
    return m_mmfile;
}
size_t Httpresponse::fileLen() const
{
    return mmFileState.st_size;
}

int Httpresponse::_code() const
{
    return m_code;
}

//获取错误页面的文件属性，将错误页面路径放入 m_path
void Httpresponse::errorHTML()
{
    if (CODE_PATH_4XX.count(m_code) > 0)
    {
        std::cout <<"Httpresponse::errorHTML() : replace 4XX html\n";
        m_path = CODE_PATH_4XX.find(m_code)->second;
        stat((m_srcDir + m_path).data(), &mmFileState);
    }
}

void Httpresponse::errorContent(Buffer &buffer, std::string message)
{
    std::string body;
    std::string status;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    if (CODE_STATUS.count(m_code) == 1)
    {
        status = CODE_STATUS.find(m_code)->second;
    }
    else
    {
        status = "Bad Request";
    }
    body += std::to_string(m_code) + " : " + status + "\r\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>HttpWebServer</em></body></html>";

    buffer.append("Content-length: " + std::to_string(body.size()) + "\r\n\r\n");
    buffer.append(body);
}