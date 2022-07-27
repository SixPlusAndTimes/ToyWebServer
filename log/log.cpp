//
// Created by 19269 on 2022/7/25.
//

#include "log.h"
Log::Log() {
    // current date/time based on current system
    time_t now = time(0);


    tm *ltm = localtime(&now);

    // print various components of tm structure.
//    std::cout << "Year:" << 1900 + ltm->tm_year<<std::endl;
//    std::cout << "Month: "<< 1 + ltm->tm_mon<< std::endl;
//    std::cout << "Day: "<< ltm->tm_mday << std::endl;
//    std::cout << "Time: "<< ltm->tm_hour << ":";
//    std::cout << ltm->tm_min << ":";
//    std::cout << ltm->tm_sec << std::endl;

    //准备日志文件名
    char filePath[64] = {0};
    snprintf(filePath,sizeof filePath,"./log_file/%d_%02d_%02d_%02d:%02d:%02d.log",
             1900 + ltm->tm_year,ltm->tm_mon + 1,ltm->tm_mday,ltm->tm_hour,ltm->tm_min,ltm->tm_sec);
    open_file_ = fopen(filePath,"w");

    if(open_file_ == nullptr)
    {
        mkdir("./log_file/",0776);
        open_file_ = fopen(filePath,"w");
    }
    // 启动日志线程
    _logThread1 = std::thread(&Log::logThreadFunc1, this, 1);
//    _logThread2 = std::thread(&Log::logThreadFunc2, this, 1);
    _logThread2 = std::thread(&Log::logThreadFunc1, this, 1);
    // 设置线程的等级
    log_level_ = 1; // 默认为1：BASE等级

}

Log::~Log() {
    fclose(open_file_);
    /*
    日志线程，只是调用这个实例的程序的一个子线程，如果调用者要结束进程了
    需要等待这个日志线程结束后再退出
    */
    if (_logThread1.joinable()) { _logThread1.join(); }
    if (_logThread2.joinable()) { _logThread2.join(); }
}

//局部单例模式： 懒汉
Log* Log::getInstance() {
    static Log log_instance_;
    return &log_instance_;
}

//生产者
void Log::writeMsg(int level, const char *filename, const char *func, int line, const char *format, ...) {
//    std::cout << "ing writeMsg\n";
    if(level >= log_level_) {
//        std::cout << "log_leve = " << level<<std::endl;
//        std::cout << "threadlevel = " <<log_level_<< std::endl;

        std::string msg;
        char date[30] = {0};
        getDate(date);
        msg.append(date);
        // msg.append(" " + std::to_string(gettid()) + " ");

        msg.append("@ ");
        msg.append(filename);
        msg.append(":" + std::to_string(line));
        switch(level) {
            case 1:
            {
                msg.append(" [BASE] ");
                break;
            }
            case 2:
            {
                msg.append(" [DEBUG] ");
                break;
            }
            case 3:
            {
                msg.append(" [INFO] ");
                break;
            }
            case 4:
            {
                msg.append(" [WARN] ");
                break;
            }
            case 5:
            {
                msg.append("ERROR ");
                break;
            }
            default:
            {
                msg.append("INFO ");
                break;
            }
        }


        va_list vaList;
        va_start(vaList, format);
        char str[256] = {0};
        vsnprintf(str, sizeof(str) - 1, format, vaList);
        va_end(vaList);
        msg.append(str);
        msg.append(" ");

//        std::cout << "msg = " << msg.c_str() << std::endl;
        // 并发访问，上锁
        {
            queue_mtx_.lock();
            while (buffer_queue_.size() >= MAX_QUEUE_SIZE)
            {
                //有界缓冲区满
                //本线程未生产者县城内，因此给fill发送信号，并在empty上阻塞
                queue_cond_fill_.signal();
                queue_cond_empty_.wait(queue_mtx_.getMutex());
            }
            buffer_queue_.push(msg);
            queue_mtx_.unlock();
        }
    }
}

// 获取时间：20220419 21:06:13.153335Z
void Log::getDate(char *date)
{
    struct timeval pt;
    gettimeofday(&pt, NULL);
    struct tm *lt = gmtime(&pt.tv_sec);
    sprintf(date, "%d%02d%02d %02d:%02d:%02d.%-6ldZ",
            lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday, (lt->tm_hour + 8) % 24, lt->tm_min, lt->tm_sec, pt.tv_usec);
}

//消费者线程等待fill信号，并发信号给empty
void Log::writeToFile() {
    queue_mtx_.lock();
    while(buffer_queue_.empty()) {

        queue_cond_fill_.wait(queue_mtx_.getMutex());
    }

    //这里bufferqueue中就已经有数据了。本线程全部消费它
    while(!buffer_queue_.empty()) {
        std::string msg(buffer_queue_.front());
        buffer_queue_.pop();
        fprintf(open_file_,"%s\r\n",msg.c_str());
        fflush(open_file_);
    }
    //发出empty信号，未生产者线程接触阻塞
    queue_cond_empty_.broadCast();

    queue_mtx_.unlock();
}


// 日志线程的工作函数
void Log::logThreadFunc1(int num)
{
   while (true)
   {
        sleep(1);
        writeToFile();
   }
}