#include "friendmodel.hpp"
#include "db.h"
//添加好友关系
void FriendModel::insert(int userid, int friendid)
{
     //1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into Friend values(%d,%d)", userid,friendid);

    MySQL mysql;
    if(mysql.connect())
    {
       mysql.update(sql);
    }

}

//返回用户的好友列表
vector<User> FriendModel::query(int userid)
{
    //1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select a.id,a.name,a.state from User a inner join Friend b on b.friendid = a.id where b.userid=%d", userid);

    vector<User> vec; //存储查询到的离线消息
    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        if(res != nullptr)
        {
            //把userid用户的离线消息存储到vec中
            MYSQL_ROW row ;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;
                user.setId(atoi(row[0])); //id
                user.setName(row[1]); //name
                user.setState(row[2]); //state
                vec.push_back(user); //存储查询到的好友信息
            }
            mysql_free_result(res); //释放结果集
            return vec; //返回查询到的离线消息  
        }
    }
    return vec; //如果没有查询到离线消息，返回空vector
}