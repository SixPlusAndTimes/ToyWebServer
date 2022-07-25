# 编译运行
~~~shell
#进入ToyWebserver目录
make all #编译
make run #运行
# 默认运行设置为:
  端口 10000
  日志等级: 1 (无debug信息)
  超时时间: -1(不进行TCP连接超时检测)
  线程数量: 8
  epoll模式: LT(ListenFD) + ET(ConnectedFDs)
~~~
> 注意 ： 服务器端口默认为 10000 
> 
> 如果要修改端口，则打开 `Makefile` 文件， 将最后一行的 10000 改成自定义端口
> 
> ![img.png](img.png)