# libnetxcq
支持TCP和UDP的网络库

### 如何配置
使用gflags实现配置项管理。配置文件在data/config文件夹中。其中listen.conf是监听模块的配置文件；connector.conf是连接模块的配置文件。在listen.conf中，communication_type指定了连接类型（TCP或者UDP），其中SOCK_STREAM对于TCP，SOCK_DGRAM对应UDP。port指定了网络连接的端口

### TCP连接
为了承载高并发的需求，网络连接的处理采用了epoll的多路复用方法。TCP连接模块的基本的驱动架构为单线程监听连接，多线程处理消息通讯。listen文件夹下是监听连接模块的代码文件。其中tcp_listen_service.h和tcp_listen_service.cpp处理TCP连接的监听。TCPListenService继承自ListenService，由一个单独的线程驱动，驱动函数为main_loop。在main_loop函数监听连接。

新加入的连接交给ConnectorManager处理。ConnectorManager创建了多个线程处理连接的消息通讯，以socket_id散列到（取余）不同的线程中。连接线程抽象为ConnectorThread对象，其中会包括多个Connector对象，每个Connector表示一个TCP连接，抽象为Socket对象。每个ConnectorThread会创建一个epoll对象，epoll对象处理所有ConnectorThread管理的连接。

为了更加安全有效的处理TCP数据流，使用环形队列处理TCP数据。具体请参考socket_input.cpp和socket_output.cpp

### UDP连接
UDP连接的处理略有不同。驱动框架为单线程监听连接，线程池处理消息并回复消息。即监听模块监听到新连接后，将内容读取后添加到线程池，线程读取、解析和执行消息命令，并回复命令。UDP模块的主要文件为udp_listen_serivice.cpp和udp_socket.cpp

### 依赖库
使用了tcmalloc优化内存分配。tcmalloc以源码的形式的嵌入到工程中，请参考libs/tcmalloc。在libs/tcmalloc文件夹，使用make命令编译tcmalloc的静态库

### 编译
在libnetxcq文件夹下，使用make命令即可编译出libnetxcq.so库文件。为了方便其他工程的使用，需要将libnetxcq.so拷贝到/usr/local/lib文件夹，将data文件夹拷贝到/usr/share/libnetxcq文件夹。执行make install完成拷贝工作

### 如何使用
example文件夹下有server和client的使用例子。分别使用make server和make client编译服务器端和客户端

### 未来工作
1. UDP模块未做更多测试。消息包大小限制，消息包读取等工作需要更加细致
2. log模块使用了简单的同步日志，后面会考虑提供一个异步日志模块
