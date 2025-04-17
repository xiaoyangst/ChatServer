#ifndef CHATSERVER_CHATSERVER_H
#define CHATSERVER_CHATSERVER_H

//#include "net/TcpServer.h"
//#include "net/EventLoop.h"
//
//class ChatServer{
//public:
//    ChatServer(EventLoop* loop,const InetAddress& listenAddr,const std::string& nameArg);
//    //开启事件循环
//    void start();
//private:
//    //连接的回调
//    void onConnection(const TcpConnectionPtr&);
//    //读写事件的回调
//    void onMessage(const TcpConnectionPtr&, Buffer*, Timestamp);
//    TcpServer _server;
//    EventLoop* _loop;
//};

#include "hv/TcpServer.h"

class ChatServer {
 public:
  ChatServer(const std::string &ip, int port,int threadNum);
  //开启事件循环
  void start();
 private:
  //连接的回调
  void onConnection(const hv::SocketChannelPtr &);
  //读写事件的回调
  void onMessage(const hv::SocketChannelPtr &, hv::Buffer *);
 private:
  unpack_setting_t *server_unpack_setting;
  hv::TcpServer _server;
};

#endif //CHATSERVER_CHATSERVER_H
