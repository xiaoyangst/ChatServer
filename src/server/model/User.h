//
// 根据数据库中对应的字段来配置，数据类型务必一致
//

#ifndef CHATSERVER_USER_H
#define CHATSERVER_USER_H

#include <string>

class User {
public:
    User(int id = -1, std::string name = "",
         std::string pwd = "", std::string state = "offline");

    void setId(const int id);

    void setName(const std::string name);

    void setPassword(const std::string pwd);

    void setState(const std::string state);

    int getId();

    std::string getName();

    std::string getPassword();

    std::string getState();

private:
    int _id;
    std::string _name;
    std::string _password;
    std::string _state;
};

#endif //CHATSERVER_USER_H
