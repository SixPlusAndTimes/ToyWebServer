//
// Created by 19269 on 2022/7/18.
//

#include "httprequest.h"
enum class Httprequest::PARSE_STATE : int {
    REQUEST_LINE,
    HEADERS,
    BODY,
    FINISH
};

enum class Httprequest::HTTP_CODE : int {
    NO_REQUEST = 0,
    GET_REQUEST,
    BAD_REQUEST,
    NO_RESOURSE,
    FORBIDDENT_REQUEST,
    FILE_REQUEST,
    INTERNAL_ERROR,
    CLOSED_CONNECTION
};

const std::unordered_set<std::string> Httprequest::DEFAULT_HTML{
        "/index", "/welcome", "/video", "/picture", "/login", "/register"};

Httprequest::Httprequest() { _init(); }

void Httprequest::_init() {
    m_content_length = 0;
    m_method = m_path = m_version = "";
    m_state = PARSE_STATE::REQUEST_LINE;
    m_header.clear();
    m_usrInfoPost.clear();
}

/*解析请求报文*/
bool Httprequest::parse(Buffer &buff) {
    printf("in Httprequest::parse(Buffer &buff)\n");
    const char CRLF[] = "\r\n";
    if(buff.readableBytes() <= 0 ) {
        printf("empty buff\n");
        return false;
    }
    while(buff.readableBytes() && m_state != PARSE_STATE::FINISH) {
        //lineEnd 指向 buff中第一个出现的'\r\n'的\r,如果没有\r\n 就返回buff.curWritePtr()
        const char *lineEnd = std::search(buff.curReadPtr(), buff.curWritePtr(), CRLF, CRLF + 2);
        std::string line(buff.curReadPtr(),lineEnd);
        switch (m_state) {
            case PARSE_STATE::REQUEST_LINE:
                if(!_parseRequestLine(line))
                {
                    return false;
                }
                //请求行解析成功，将 m_path改成本服务器资源的绝对路径
                _parsePath();
                break;
            case PARSE_STATE::HEADERS:
                _parseRequestHeader(line);

                if(buff.readableBytes() <= 2) {
                    //2 是换行符的大小
                    m_state = PARSE_STATE::FINISH;
                }
                break;

            case PARSE_STATE::BODY:
                _parseDataBody(line);
                _getPost();
                break;
            default:
                std::cout<<"主状态机 未知状态\n";
                break;
        }

        if(lineEnd == buff.curWritePtr()) {
            //表示buf中没有\r\n 还没有接受到一行
            break;//退出while循环让主程序继续读对端数据
        }
        //表示已经读过一行了
        buff.updateReadPtr(lineEnd - buff.curReadPtr() + 2);
    }
    return true;
}

bool Httprequest::_parseRequestLine(const std::string &line) {
    char* requestLine = const_cast<char *>(line.c_str());
    //找url的开头
    char* url = strpbrk(requestLine," \t");
    if(! url) return false;
    *url++ = '\0';

    //解析method
    char* method = requestLine;
    if(strcasecmp(method,"GET") == 0) {
        m_method = "GET";
    } else if(strcasecmp(method,"POST") == 0) {
        m_method = "POST";
    } else {
        return false;
    }
    //找版本号开头,并解析版本号
    char* version = strpbrk( url, " \t" );
    if(!version) {return false;}
    *version++ = '\0';
    m_version = std::string(version);

    //保存url
    m_path = std::string(url);
    //改变状态机状态
    m_state = PARSE_STATE::HEADERS;

    printf("m_method:%s, m_path:%s, m_version:%s\n", m_method.c_str(), m_path.c_str(), m_version.c_str());
    return true;
}

void Httprequest::_parsePath() {
    if(m_method == "GET"){
        if (m_path == "/")
        {
            // 访问首页,自动跳转到固定资源位置
            m_path = "/index.html";
        }
        else
        {
            // 查找
            for (auto &item : DEFAULT_HTML)
            {
                if (item == m_path)
                {
                    m_path += ".html";
                    break;
                }
            }
        }
    }
}

void Httprequest::_parseRequestHeader(const std::string &line) {
    char* requestLine = const_cast<char *>(line.c_str());
    if(requestLine[0] == '\0') {
        //空行，将状态改为解析请求体
        //if(m_content_length != 0) {
            m_state = PARSE_STATE::BODY;
            return;
        //}
    }

    char* tokenIndex = requestLine;
    while(*tokenIndex != ':') {
        tokenIndex++;
    }
    *tokenIndex = '\0';
    tokenIndex++;
    m_header[std::string(requestLine)] = std::string(tokenIndex);
    printf("header=%s:%s\n", std::string(requestLine).c_str(), m_header[std::string(requestLine)].c_str());
}

void Httprequest::_parseDataBody(const std::string &line) {
    m_body = line;
    if(m_method == "POST")
    {
        _parsePost();
    }
    m_state = PARSE_STATE::FINISH;
}

void Httprequest::_parsePost() {

}
void Httprequest::_getPost() {}

std::string Httprequest::path() const
{
    return m_path;
}
std::string Httprequest::method() const
{
    return m_method;
}
std::string Httprequest::version() const
{
    return m_version;
}

bool Httprequest::isKeepAlive() const
{
    if (m_header.count("Connection") > 0)
    {
        // LOG_DEBUG("Connection:%s, version:%s", m_header.find("Connection")->second.c_str(), m_version.c_str())
        std::string connection = m_header.find("Connection")->second;
        if(m_version == "HTTP/1.1" && (connection == "keep-alive" || connection == "Keep-Alive") ){
            return true;
        }
    }
    return false;
}