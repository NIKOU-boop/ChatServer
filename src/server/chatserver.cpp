#include "chatserver.hpp"
#include "json.hpp"
#include "chatservice.hpp"

#include <functional>
#include <string>
using namespace std::placeholders;
using namespace std;
using json = nlohmann::json;

ChatServer::ChatServer(EventLoop *loop, const InetAddress &listenAddr)
    : _server(loop, listenAddr, "ChatServer"), _loop(loop)
{
    //设置连接回调函数
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this,_1));
    //设置消息回调函数
    _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));
    // 设置线程池的线程数
    _server.setThreadNum(4); 
}

//启动聊天服务器
void ChatServer::start()
{
    //启动聊天服务器
    _server.start();
}
//连接回调函数
void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    //客户端断开连接
    if(!conn->connected())
    {
        //从在线用户的连接信息中删除该用户
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown(); 
    }
}
//消息回调函数
void ChatServer::onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time)
{
    string buf = buffer->retrieveAllAsString();
    //数据的反序列化
    json js = json::parse(buf);
    //达到目的：完全解耦网络模块和业务模块的代码
    //通过js["msgid"]获取业务handler
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    //回调消息绑定的好的事件处理器，来执行相应的业务处理
    msgHandler(conn, js, time); 
}   