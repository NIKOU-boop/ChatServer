#include "chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>
#include <vector>
using namespace std;
using namespace muduo;


// ChatService类的单例模式实现
ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}

//注册消息以及对应的Handler回调
ChatService::ChatService()
{
    //用户基本业务管理相关事件处理回调注册
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({LOGINOUT_MSG,std::bind(&ChatService::loginout, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
    //用户群组相关事件处理回调注册
    _msgHandlerMap.insert({CREATE_GROUP_MSG,std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG,std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG,std::bind(&ChatService::groupChat, this, _1, _2, _3)});
    //连接redis服务器
    if(_redis.connect())
    {
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));
    }
}

//服务器异常，业务重置方法
void ChatService::reset()
{
    _userModel.resetState(); //重置用户状态信息
}
MsgHandler ChatService::getHandler(int msgid)
{
    //记录错误日志，msgid没有对应的事件处理回调
    auto it = _msgHandlerMap.find(msgid);
    if(it == _msgHandlerMap.end())
    { 
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp)
        {
            LOG_ERROR << "消息类型: " << msgid << " 没有对应的处理器";
        };
    }
    else
    {
        return _msgHandlerMap[msgid]; 
    }   
}

// 处理登录逻辑
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"].get<int>();
    string password = js["password"];

    User user = _userModel.query(id);
    if(user.getId() == id && user.getPassword() == password)
    {
        if(user.getState() == "online")
        {
            //用户已经在线，不允许重复登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["error"] = 2; //2表示用户已经在线
            response["errmsg"] = "用户已经在线,请重新输入新账号";
            conn->send(response.dump());
            return;
        }
        else
        {
            // 用户登录成功，更新在线用户的连接信息
            {
                lock_guard<mutex> lock(_connMutex);
                _userConnMap.insert({id, conn});
            }

            //用户登陆成功后，向redis订阅channel(id)
            _redis.subscribe(id);

            //登录成功，更新用户状态信息，返回用户信息
            user.setState("online");
            _userModel.updateState(user);
            

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["error"] = 0; //0表示成功
            response["id"] = user.getId();
            response["name"] = user.getName();

            //查询该用户是否有离线消息
            vector<string> vec = _offlineMsgModel.query(id);
            if(!vec.empty())
            {
                response["offlinemsg"] = vec; //存储离线消息
                //读取离线消息后，删除该用户的离线消息
                _offlineMsgModel.remove(id);
            }
            //查询该用户的好友列表
            vector<User> userVec = _friendModel.query(id);
            if(!userVec.empty())
            {
                vector<string> vec2;
                for(User &user : userVec)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2; //存储好友列表
            }
            //查询该用户的群组信息
            vector<Group> groupuserVec = _groupModel.queryGroups(id);
            if(!groupuserVec.empty())
            {
                vector<string> vec3;
                for(Group &group : groupuserVec)
                {
                    json grpjs;
                    grpjs["id"] = group.getId();
                    grpjs["groupname"] = group.getName();
                    grpjs["groupdesc"] = group.getDesc();
                    vector<string> vec4;
                    for(GroupUser &user : group.getUsers())
                    {
                        json userjs;
                        userjs["id"] = user.getId();
                        userjs["name"] = user.getName();
                        userjs["state"] = user.getState();
                        userjs["role"] = user.getRole();
                        vec4.push_back(userjs.dump());
                    }
                    grpjs["users"] = vec4;
                    vec3.push_back(grpjs.dump());
                }
                response["groups"] = vec3; //存储好友列表
            }
            conn->send(response.dump());                                  
        }

    }
    else  
    {
        //该用户不存在登录失败，返回错误信息
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["error"] = 1; //1表示失败
        response["errmsg"] = "用户名或密码错误";
        conn->send(response.dump());
    }
}

 // 处理注册逻辑
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    string name = js["name"];
    string password = js["password"];

    User user;
    user.setName(name);
    user.setPassword(password);

    bool state = _userModel.insert(user);
    if(state)
    {
        //注册成功，返回用户信息
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["error"] = 0; //0表示成功
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else
    {
        //注册失败，返回错误信息
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["error"] = 1; //1表示失败
        conn->send(response.dump());
    }
    
}

//处理注销业务
void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();

    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if(it != _userConnMap.end())
        {
            _userConnMap.erase(it);
        }
    }
    //用户注销，在redis取消订阅通道
    _redis.unsubscribe(userid);

    //更新用户状态信息为离线
    User user(userid ,"","","offline");
    _userModel.updateState(user);
}

// 处理客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    {
        lock_guard<mutex> lock(_connMutex);
        for(auto it = _userConnMap.begin(); it != _userConnMap.end();++it)
        {
            if(it->second == conn)
            {
                //从在线用户的连接信息中删除该用户
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }

    //用户注销，在redis取消订阅通道
    _redis.unsubscribe(user.getId());

    //如果用户id不为-1，说明该用户在线
    if(user.getId() != -1)
    {
        //更新用户状态信息为离线
        user.setState("offline");
        _userModel.updateState(user);
    }
}

//一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toid = js["toid"].get<int>();
    //用户在本服务器上，且在线
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toid);
        if(it != _userConnMap.end())
        {
            //用户在线,转发消息,服务器主动推送消息给toid用户
            it->second->send(js.dump());
            return;
        }
    }

    //用户在其他服务器上，且在线
    User user = _userModel.query(toid);
    if(user.getState() == "online")
    {
        _redis.publish(toid,js.dump());
        return;
    }

    //toid用户不在线,存储离线消息
    _offlineMsgModel.insert(toid, js.dump()); 
}

//添加好友业务
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    //存储好友信息
    _friendModel.insert(userid, friendid);
}

void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    //创建群组
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    //存储新创建的群组信息
    Group group(-1,name,desc);
    if(_groupModel.createGroup(group))
    {
        //创建成功，存储创建人的信息
        _groupModel.addGroup(userid, group.getId(), "creator");
    }
}

//加入群组业务
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    //加入群组
    int userid = js["id"].get<int>();
    int groupid = js["groupid"];
    //存储用户加入群组的信息
    _groupModel.addGroup(userid, groupid, "normal");
}
//群组聊天业务
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"];
    //查询群组用户列表
    vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);

    lock_guard<mutex> lock(_connMutex);
    for(int id : useridVec)
    {
        auto it = _userConnMap.find(id);
        if(it != _userConnMap.end())
        {
            //在线用户，推送消息
            it->second->send(js.dump());
        }
        else
        {
            //用户在其他服务器上，且在线
            User user = _userModel.query(id);
            if(user.getState() == "online")
            {
                _redis.publish(id,js.dump());
            }
            else
            {
                //离线用户，存储离线消息
                _offlineMsgModel.insert(id, js.dump());
            }
           
        }
    }
}


//redis消息
void ChatService::handleRedisSubscribeMessage(int id,string message)
{
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(id);
    if(it != _userConnMap.end())
    {
        it->second->send(message);
        return;
    }
    //用户不在线,存储离线消息
    _offlineMsgModel.insert(id, message); 

}