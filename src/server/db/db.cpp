#include "db.h"
#include <muduo/base/Logging.h>

//数据库配置信息
static string server = "127.0.0.1"; //数据库地址
static string user = "root"; //用户名
static string password = "123456"; //密码
static string dbname = "chat"; //数据库名

//初始化数据库连接
MySQL::MySQL()
{
    _conn = mysql_init(nullptr);
}

//释放数据库连接资源
MySQL::~MySQL()
{
    if(_conn != nullptr)
    {
        mysql_close(_conn);
    }
}
    
//连接数据库
bool MySQL::connect()
{
    MYSQL *p = mysql_real_connect(_conn,
                                  server.c_str(), //数据库地址
                                  user.c_str(),      //用户名
                                  password.c_str(),  //密码
                                  dbname.c_str(),      //数据库名
                                  3306,        //端口号
                                  nullptr,     //unix socket
                                  0);          //客户端标志
    if(p != nullptr)
    {
        //C和C++代码默认编码字符是ASCII，如果不设置，从Mysql上拉下来的代码显示为？
        mysql_set_character_set(_conn, "set names gbk");
        LOG_INFO << "连接数据库成功！"; 
    }
    else
    {
        LOG_ERROR << "连接数据库失败！";
    }
    return p ;
}
    
//更新操作
    
bool MySQL::update(string sql)
{
    if(mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
        << sql << "更新失败！";
        return false;
    }
    return true;
}
    
//查询操作
MYSQL_RES* MySQL::query(string sql)
{
    if(mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
        << sql << "查询失败！";
        return nullptr;
    }
    //返回查询结果集
    return mysql_store_result(_conn);
}

//获取连接
MYSQL* MySQL::getConnection()
{
    return _conn;
}