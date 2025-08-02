#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include "json.hpp"
#include "redis.hpp"
#include <muduo/net/TcpServer.h>
#include <unordered_map>
#include <functional>
#include <mutex>
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
using namespace std;
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;

using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp)>;
//聊天服务器业务类
class ChatService
{
public:
    //单例模式获取聊天服务器业务对象
    static ChatService *instance();     
    //处理聊天业务
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //处理注册业务
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //一对一聊天业务
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //添加好友业务
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //创建群组业务
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //加入群组业务
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //群组聊天业务
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //处理注销业务
    void loginout(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //redis消息
    void handleRedisSubscribeMessage(int id,string message);
    //服务器异常，业务重置方法
    void reset();
    //处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);
    //获取消息对应的处理器
    MsgHandler getHandler(int msgid);

private:
    ChatService();

    //存储消息id和业务处理方法的映射关系
    unordered_map<int, MsgHandler> _msgHandlerMap;

    //存储在线用户的通信连接
    unordered_map<int, TcpConnectionPtr> _userConnMap;

    //定义一把互斥锁，保证对_userConnMap的线程安全访问
    mutex _connMutex;

    //用户数据操作对象
    UserModel _userModel;
    offlineMsgModel _offlineMsgModel;
    FriendModel _friendModel;
    GroupModel _groupModel;

    //redis操作对象
    Redis _redis;
};
#endif