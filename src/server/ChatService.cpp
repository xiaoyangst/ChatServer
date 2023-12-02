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

//用户提供账号和密码
void ChatService::registerHandler(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time) {
    LOG_DEBUG << "start do register !";

    std::string name = js["name"];
    std::string pwd = js["pwd"];

    User user;
    user.setName(name);
    user.setPassword(pwd);

    bool state = _userModel.insert(user);   //记录插入是否成功的状态保存到state变量中，true   /   false

    if (state){ //注册成功
        json response;
        response["msgid"] = REGISTER_MSG;
        response["errno"] = 0;
        response["id"] = user.getId();

        //dump方法把json转换为std::string
        conn->send(response.dump());
    } else{
        json response;
        response["msgid"] = REGISTER_MSG_ACK;
        response["errno"] = 1;
        // 注册已经失败，不需要在json返回id
        conn->send(response.dump());
    }

}

void ChatService::loginHandler(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time) {
    LOG_DEBUG << "start do login !";

    int id = js["id"].get<int>();
    std::string pwd = js["pwd"];

    User user = _userModel.qurry(id);
    if (user.getId() == id && user.getPassword() == pwd){
        if (user.getState() == "online"){
            //不可重复登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "this account is using, input another!";
            conn->send(response.dump());
        } else{ //登录成功
            user.setState("online");
            _userModel.updateState(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();

            conn->send(response.dump());
        }
    }else{  //登录失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = -1;
        response["errmsg"] = "login failed ,Try again!\n";
        conn->send(response.dump());
    }
}