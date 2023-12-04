//
// 存储某个用户的离线信息
//

#ifndef CHATSERVER_OFFLINEMSGMODEL_H
#define CHATSERVER_OFFLINEMSGMODEL_H

#include <string>
#include <vector>

class OfflineMsgModel{
public:
    //存储用户离线信息
    void insert(int userId,std::string msg);
    //移除用户离线信息
    void remove(int userId);
    //查询用户离线信息
    std::vector<std::string> query(int userId);
};

#endif //CHATSERVER_OFFLINEMSGMODEL_H
