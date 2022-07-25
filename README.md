# 编译运行
~~~shell
#进入ToyWebserver目录
make all #编译
make run #默认配置运行
# 默认运行设置为:
  端口 10000
  日志等级: 1 (无debug信息)
  超时时间: -1(不进行TCP连接超时检测)
  线程数量: 8
  epoll模式: LT(ListenFD) + ET(ConnectedFDs)
~~~

个性化运行
~~~shell
cd bin;
./myserver [-p port] [-l loglevel] [-m TRIGMode] [-s sql_num] [-t thread_num]
~~~

- -p 自定义端口号
  - 默认 10000
- -l 设置日志等级
  - 默认为1，即不开启日志
  - 2 表示 DEBUG等级
  - 3 表示 INFO等级
  - 4 表示 WARN等级
  - 5 表示 ERROR等级
- -m listenfd和connfd的模式组合
  - 默认使用LT + LT
- -s，数据库连接数量
  - 默认为8
- -t 服务器处理http连接的线程数量
  - 默认为8