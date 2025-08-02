#ifndef GROUP_H
#define GROUP_H

#include "groupuser.hpp"
using namespace std;
#include <vector>
#include <string>


class Group
{
public:
    Group(int id = -1,string name = "", string desc = "")
    {
        this->id = id;
        this->name = name;
        this->desc = desc;
    }

    void setId(int id) { this->id = id; }
    void setName(string name) { this->name = name; }
    void setDesc(string desc) { this->desc = desc; }

    int getId() const { return this->id; }
    string getName() const { return this->name; }
    string getDesc() const { return this->desc; } 
    vector<GroupUser> &getUsers() { return this->users; } //获取群组用户列表

private:
    int id; //群组id
    string name; //群组名称        
    string desc; //群组描述
    vector<GroupUser> users; //群组用户列表
};



#endif