#include "ChatServer.h"
#include "ChatService.h"
#include "json.hpp"
#include <functional>
#include <string>
#include "net/Logger.h"

using namespace std;
using namespace placeholders;
using json = nlohmann::json;

ChatServer::ChatServer(EventLoop *loop, const InetAddress &listenAddr,
                       const std::string &nameArg)
        : _loop(loop), _server(loop, listenAddr, nameArg) {
    //注册连接回调
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this,_1));
    //注册消息事件回调
    _server.setMessageCallback(std::bind(&ChatServer::onMessage, this,_1,_2,_3));

    //设置subLoop线程数量
    _server.setThreadNum(4);
}

void ChatServer::onConnection(const TcpConnectionPtr &conn) {
    if (!conn->connected()){    //用户断开连接
        LOG_INFO("处理用户异常断开");
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}

void ChatServer::onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time) {
    //获取客户端发送的信息
    string buf = buffer->retrieveAllAsString();
    //反序列化
    json js = json::parse(buf);

    // 完全解耦网络模块和业务模块
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());

    msgHandler(conn,js,time);
}

void ChatServer::start() {
    _server.start();
}
