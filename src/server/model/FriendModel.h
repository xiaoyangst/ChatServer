//
// 好友列表
//

#ifndef CHATSERVER_FRIENDMODEL_H
#define CHATSERVER_FRIENDMODEL_H
#include <vector>
#include "User.h"
class FriendModel{
public:
    //添加好友
    void insert(int userId, int friendId);
    //查询好友列表
    //查询结果为User，总不能只查出个用户ID，得是完整的用户信息
    std::vector<User> query(int userId);
};

#endif //CHATSERVER_FRIENDMODEL_H
