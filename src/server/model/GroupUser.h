//
// 用户所在群组的信息
//

#ifndef CHATSERVER_GROUPUSER_H
#define CHATSERVER_GROUPUSER_H

#include "User.h"
#include <string>

class GroupUser : public User{
public:
    GroupUser() = default;
    void setRole(const std::string& role){
        _role = role;
    }
    std::string getRole(){
        return _role;
    }
private:
    std::string _role;
};

#endif //CHATSERVER_GROUPUSER_H
