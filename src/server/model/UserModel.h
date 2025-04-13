//
// 用户表相关操作，因此就是对User.h操作
//

#ifndef SRC_USERMODEL_H
#define SRC_USERMODEL_H

#include "User.h"

class UserModel{
public:
    //插入
    bool insert(User &user);

    //查询
    User qurry(int id);

    //更新用户状态
    bool updateState(User user);

    //重置用户的状态信息
    void resetState();
};

#endif //SRC_USERMODEL_H
