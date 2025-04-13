//
// 群
//

#ifndef CHATSERVER_GROUP_H
#define CHATSERVER_GROUP_H

#include <string>
#include <vector>
#include "GroupUser.h"


class Group{
public:
    explicit Group(int id = -1, std::string name = "",std::string desc = "");

    void setId(int id);
    void setName(std::string name);
    void setDesc(std::string desc);

    int getId() const;
    std::string getName();
    std::string getDesc();
    std::vector<GroupUser> &getUsers();
private:
    int _id;
    std::string _name;
    std::string _desc;
    std::vector<GroupUser> _users;
};

#endif //CHATSERVER_GROUP_H
