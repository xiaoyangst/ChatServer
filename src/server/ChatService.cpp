#include "ChatService.h"


#include "public.h"
#include <iostream>
#include <muduo/base/Logging.h>

//注册消息以及对应的Handle对应的回调函数
ChatService::ChatService() {
    _msgHandlerMap.insert({LOGIN_MSG,std::bind(&ChatService::loginHandler, this,_1,_2,_3)});
    _msgHandlerMap.insert({REGISTER_MSG,std::bind(&ChatService::registerHandler, this,_1,_2,_3)});
    _msgHandlerMap.insert({ ONE_CHAT_MSG,std::bind(&ChatService::oneChatHandler, this,_1,_2,_3)});
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

        LOG_INFO <<name<<":register success!";

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

            LOG_INFO <<user.getName()<<":login success!";

            {
                lock_guard<mutex> lockGuard(_conmutex);
                _userConnMap.insert({id,conn});
            }

            user.setState("online");
            _userModel.updateState(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();

            //查看是否有离线消息
            std::vector<std::string> result = _offlineMsgModel.query(id);
            if (!result.empty()){
                //有离线消息
                response["offlinemsg"] = result;
                //读取之后就该移除离线消息
                _offlineMsgModel.remove(id);
            }else{
                LOG_INFO << "no offlienmsg";
            }

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

void ChatService::clientCloseException(const muduo::net::TcpConnectionPtr &conn) {
    User user;
    {   //找到连接，并在 _userConnMap 删除它
        lock_guard<mutex> lockGuard(_conmutex);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end() ; ++it) {
            if (it->second == conn){
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }

    //更新用户状态
    if (user.getId() != -1){
        user.setState("offline");
        _userModel.updateState(user);
    }
}

//如果用户登录状态，直接给他发消息
//如果用户离线状态，把离线消息存储起来，等他上线时候读取
void ChatService::oneChatHandler(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    //先获取 聊天对象的ID
    int toId = js["toid"].get<int>();

    {
        lock_guard<mutex> lockGuard(_conmutex);
        auto it = _userConnMap.find(toId);
        if (it != _userConnMap.end()){  //确认在线
            it->second->send(js.dump());
            return;
        }
    }

    //处理离线
    _offlineMsgModel.insert(toId,js.dump());
}

void ChatService::reset() {
    //全部online转换为offline
    _userModel.resetState();
}


