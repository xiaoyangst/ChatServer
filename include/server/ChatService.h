#ifndef CHATSERVER_CHATSERVICE_H
#define CHATSERVER_CHATSERVICE_H

#include <unordered_map>
#include <muduo/net/TcpConnection.h>
#include "json.hpp"


using json = nlohmann::json;
using namespace std;
using namespace muduo;
using namespace muduo::net;

//业务的回调函数
using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp time)>;

class ChatService : noncopyable{
public:
    //单例模式
    static ChatService* instance(){
        static ChatService service;
        return &service;
    }

    //获取对应消息的处理器
    MsgHandler getHandler(int msdId);

    // 登录业务
    void loginHandler(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 注册业务
    void registerHandler(const TcpConnectionPtr &conn, json &js, Timestamp time);
private:
    ChatService();
    //存储消息id和其对应的事件处理方法
    unordered_map<int,MsgHandler> _msgHandlerMap;
};

#endif //CHATSERVER_CHATSERVICE_H
