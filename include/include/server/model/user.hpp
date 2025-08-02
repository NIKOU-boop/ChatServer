#ifndef USER_H
#define USER_H

#include <string>
using namespace std;

//User表的ORM类
class User
{
public:
    User(int id=-1, string name="", string pwd="", string state="offline")
    {
        this->id = id;
        this->name = name;
        this->password = pwd;
        this->state = state;
    }

    void setId(int id){this->id = id;}
    void setName(string name){this->name = name;}
    void setPassword(string pwd){this->password = pwd;} 
    void setState(string state){this->state = state;}

    int getId() const {return id;}
    string getName() const {return name;}
    string getPassword() const {return password;}
    string getState() const {return state;}

private:
    int id; //用户id
    string name; //用户名
    string password; //密码
    string state; //用户状态

};
#endif