#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include "group.hpp"
#include <vector>
#include <string>
using namespace std;

class GroupModel
{
public:
    //创建群组
    bool createGroup(Group &group);  
    //加入群组  
    void addGroup(int userid, int groupid, string role);  
    //查询用户所在群组列表
    vector<Group> queryGroups(int userid);
    //根据群组id查询群组用户列表
    vector<int> queryGroupUsers(int userid,int groupid); 
};
#endif