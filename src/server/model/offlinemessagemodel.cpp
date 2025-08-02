#include "offlinemessagemodel.hpp"
#include "db.h"

//存储用户的离线消息
void offlineMsgModel::insert(int userid, string msg)
{
   //1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into OfflineMessage values(%d, '%s')", userid, msg.c_str());

    MySQL mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }

}
//删除用户的离线消息
void offlineMsgModel::remove(int userid)
{   //1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "delete from OfflineMessage where userid = %d", userid);

    MySQL mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }
}
//查询用户的离线消息
vector<string> offlineMsgModel::query(int userid)
{
  //1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select message from OfflineMessage where userid = %d", userid);

    vector<string> vec; //存储查询到的离线消息
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
                vec.push_back(row[0]); //row[0]是离线消息
            }
            mysql_free_result(res); //释放结果集
            return vec; //返回查询到的离线消息  
        }

    }
    return vec; //如果没有查询到离线消息，返回空vector

}