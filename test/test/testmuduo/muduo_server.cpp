/*
muduo网络库给用户提供了两个主要的类
TcpServer和TcpClient
TcpServer用于实现服务器端的功能
TcpClient用于实现客户端的功能
*/

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <functional>
#include <string>
using namespace muduo;
using namespace muduo::net;
using namespace std;
using namespace placeholders;

/*基于muduo网络库开发服务器程序
1.组合TcpServer对象
2.创建EventLoop事件循环对象的指针
3.明确TcpServer构造函数需要什么参数，输出ChatServer类的构造函数
4.在ChatServer类的构造函数中，给TcpServer对象注册用户连接创建和断开回调函数
5.设置合适的服务端线程数量，muduo库会自己分配IO线程和worker线程
*/
class ChatServer
{
public:
    ChatServer(EventLoop *loop,  //事件循环
        const InetAddress &listenAddr, //ip+port
        const string &nameArg) //服务器名称
        : server(loop, listenAddr, nameArg),_loop(loop)
        {
            //给服务器注册用户连接的创建和断开回调函数
            server.setConnectionCallback(
                std::bind(&ChatServer::onConnection, this, _1)); 
            //给服务器注册用户读写事件回调
            server.setMessageCallback(
                std::bind(&ChatServer::onMessage, this, _1, _2, _3)); 
            //设置服务器的线程数 1个IO线程 3个worker线程
            server.setThreadNum(4); 
        }
    //开启事件循环
    void start()
    {
        server.start();      
    }

private:
    //用户连接创建或断开回调
    void onConnection(const TcpConnectionPtr &conn) 
    {
        if(conn->connected()) //如果连接成功
        {
            cout << "new connection [" << conn->name() << "] from "
                 << conn->peerAddress().toIpPort() << endl;
        }
        else //连接断开
        {
            cout << "connection [" << conn->name() << "] is down"
                 << " at " << conn->peerAddress().toIpPort() << endl;
            conn->shutdown(); //关闭连接

        }
    }
    //用户读写事件回调
    void onMessage(const TcpConnectionPtr &conn, //连接
                            Buffer *buffer, //缓冲区
                            Timestamp time) //接受到数据的时间信息
    {
        string buf = buffer->retrieveAllAsString(); //获取缓冲区数据
        cout << "recv data:" << buf << "time:" << time.toString() << endl;
        //回显数据给客户端
        conn->send(buf); //将数据发送给客户端
    }
    EventLoop *_loop; // 事件循环对象
    TcpServer server; // TcpServer对象
};

int main()
{
    EventLoop loop; //创建事件循环对象epoll
    InetAddress Addr("127.0.0.1",8888); //监听地址和端口
    ChatServer server(&loop, Addr, "ChatServer"); //创建服务器对象

    server.start(); //listenfd epoll_ctl添加到epoll
    loop.loop(); //epoll_wait以阻塞方式等待新用户连接，已连接用户的读写事件等

    return 0;
}