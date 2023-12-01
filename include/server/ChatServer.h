#ifndef CHATSERVER_CHATSERVER_H
#define CHATSERVER_CHATSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;

class ChatServer{
public:
    ChatServer(EventLoop* loop,const InetAddress& listenAddr,const std::string& nameArg);
    //开启事件循环
    void start();
private:
    //连接的回调
    void onConnection(const TcpConnectionPtr&);
    //读写事件的回调
    void onMessage(const TcpConnectionPtr&, Buffer*, Timestamp);
    TcpServer _server;
    EventLoop* _loop;
};

#endif //CHATSERVER_CHATSERVER_H
