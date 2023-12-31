//
// Created by xy on 2023-11-30.
//

#ifndef CHATSERVER_PUBLIC_H
#define CHATSERVER_PUBLIC_H

// server和client的公共头文件
enum EnMsgType
{
    LOGIN_MSG = 1,  // 登录消息
    LOGIN_MSG_ACK,     // 登录响应消息
    LOGINOUT_MSG,      // 注销消息
    REGISTER_MSG,      // 注册消息
    REGISTER_MSG_ACK,  // 注册响应消息
    ONE_CHAT_MSG,      // 聊天消息
    ADD_FRIEND_MSG,    // 添加好友消息

    CREATE_GROUP_MSG,  // 创建群组
    ADD_GROUP_MSG,     // 加入群组
    GROUP_CHAT_MSG,    // 群聊天
};

enum ErrorCode
{

    PASSWORD_ERROR = 2000,
    NO_EXIST_USER,
    NO_EXIST_Group
};

#endif //CHATSERVER_PUBLIC_H
