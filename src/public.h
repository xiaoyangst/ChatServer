//
// Created by xy on 2023-11-30.
//

#ifndef CHATSERVER_PUBLIC_H
#define CHATSERVER_PUBLIC_H

// server和client的公共头文件
enum EnMsgType
{
    LOGIN_MSG = 1,  // 登录消息
    LOGIN_MSG_ACK = 2,     // 登录响应消息
    LOGINOUT_MSG = 3,      // 注销消息
    REGISTER_MSG = 4,      // 注册消息
    REGISTER_MSG_ACK = 5,  // 注册响应消息
    ONE_CHAT_MSG = 6,      // 聊天消息
	ONE_CHAT_MSG_ACK = 7,  // 聊天响应消息
    ADD_FRIEND_MSG = 8,    // 添加好友消息

    CREATE_GROUP_MSG = 9,  // 创建群组
    ADD_GROUP_MSG = 10,     // 加入群组
    GROUP_CHAT_MSG = 11, // 群聊天
};

enum ErrorCode
{

    PASSWORD_ERROR = 2000,
    NO_EXIST_USER,
    NO_EXIST_Group
};

#endif //CHATSERVER_PUBLIC_H
