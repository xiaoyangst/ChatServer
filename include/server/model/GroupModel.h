//
// 对群组的操作
//

#ifndef CHATSERVER_GROUPMODEL_H
#define CHATSERVER_GROUPMODEL_H

#include "Group.h"
#include <string>
#include <vector>

class GroupModel{
public:
    //创建群租
    bool createGroup(Group &group);
    //加入群租
    void addGroup(int userid, int groupid, std::string role);
    //查询用户所在群组信息
    std::vector<Group> queryGroups(int userid);
    //根据指定的groupid查询群组用户id列表，除userid自己，主要用于群聊业务给群组其它成员群发消息
    std::vector<int> queryGroupUsers(int userid, int groupid);
};

#endif //CHATSERVER_GROUPMODEL_H
