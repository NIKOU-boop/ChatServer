#include "chatserver.hpp"
#include "chatservice.hpp"
#include <iostream>
#include <signal.h>
using namespace std;

//处理服务器强制退出时，重载user的状态信息
void resetHandler(int)
{
    ChatService::instance()->reset();
    exit(0);
}
int main(int argc,char * argv[])
{
    char *ip =  argv[1];
    uint16_t port = atoi(argv[2]);
    signal(SIGINT,resetHandler); // 注册信号处理函数
    EventLoop loop;
    // 监听端口6000
    InetAddress listenAddr(ip,port); 
    ChatServer server(&loop, listenAddr);

    server.start();
    loop.loop(); // 启动事件循环
    
    return 0;
}