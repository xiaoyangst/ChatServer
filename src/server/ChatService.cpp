#include "ChatService.h"
#include "public.h"
#include <iostream>
#include <muduo/base/Logging.h>

//注册消息以及对应的Handle对应的回调函数
ChatService::ChatService() {
    _msgHandlerMap.insert({LOGIN_MSG,std::bind(&ChatService::loginHandler, this,_1,_2,_3)});
    _msgHandlerMap.insert({REGISTER_MSG,std::bind(&ChatService::registerHandler, this,_1,_2,_3)});
}

MsgHandler ChatService::getHandler(int msgId) {
    auto it = _msgHandlerMap.find(msgId);
    if (it != _msgHandlerMap.end()){
        return _msgHandlerMap[msgId];
    } else{
        //返回值本质上是一个函数，因此这里我们就用lambad帮我们实现并返回，只是简单的提示
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp) {
            LOG_ERROR << "msgId: " << msgId << " can not find handler!";
        };
    }
}

void ChatService::registerHandler(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time) {
    std::cout << "Register...." << std::endl;
}

void ChatService::loginHandler(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time) {
    std::cout << "Login...." << std::endl;
}