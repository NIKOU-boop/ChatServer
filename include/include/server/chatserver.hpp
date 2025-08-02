#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;

class ChatServer
{
public:
    //初始化聊天服务器对象
    ChatServer(EventLoop *loop, const InetAddress &listenAddr);
    //启动聊天服务器
    void start();
private:
    //连接回调函数
    void onConnection(const TcpConnectionPtr &conn);
    //消息回调函数
    void onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time);
    //聊天服务器对象
    TcpServer _server;
    //事件循环对象
    EventLoop *_loop;
};
#endif