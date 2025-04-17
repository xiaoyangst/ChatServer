#ifndef CHATSERVER_CHATSERVICE_H
#define CHATSERVER_CHATSERVICE_H

#include <unordered_map>
#include <mutex>
#include "hv/TcpServer.h"
#include "json.hpp"
#include "utils/nocopyable.h"
#include "UserModel.h"
#include "OfflineMsgModel.h"
#include "FriendModel.h"
#include "GroupModel.h"
#include "redispool/RedisConnectionPool.h"

using json = nlohmann::json;
using namespace std;

//业务的回调函数
using MsgHandler = std::function<void(const hv::SocketChannelPtr &conn, json &js)>;

class ChatService : noncopyable {
 public:
  //单例模式
  static ChatService *instance() {
	  static ChatService service;
	  return &service;
  }

  //获取对应消息的处理器
  MsgHandler getHandler(int msdId);

  // 登录业务
  void loginHandler(const hv::SocketChannelPtr &conn, json &js);
  // 注册业务
  void registerHandler(const hv::SocketChannelPtr &conn, json &js);
  //用户异常退出
  void clientCloseException(const hv::SocketChannelPtr &conn);
  //用户注销
  void clientLogout(const hv::SocketChannelPtr &conn, json &js);
  //点对点聊天业务
  void oneChatHandler(const hv::SocketChannelPtr &conn, json &js);
  //服务端异常退出
  void reset();
  //添加好友业务
  void addFriendHandler(const hv::SocketChannelPtr &conn, json &js);
  //创建群组业务
  void createGroup(const hv::SocketChannelPtr &conn, json &js);
  //加入群组业务
  void addGroup(const hv::SocketChannelPtr &conn, json &js);
  //群聊业务
  void chatGroup(const hv::SocketChannelPtr &conn, json &js);
  //redis订阅消息触发的回调函数
  void redis_subscribe_message_handler(const string &channel, const string &message);
 private:
  ChatService();
  //存储消息id和其对应的事件处理方法
  unordered_map<int, MsgHandler> _msgHandlerMap;

  //存储在线用户的通信连接
  unordered_map<int, hv::SocketChannelPtr> _userConnMap;

  mutex _conmutex;

  //redis操作对象
  // Redis _redis;



  UserModel _userModel;
  OfflineMsgModel _offlineMsgModel;
  FriendModel _friendModel;
  GroupModel _groupModel;

};

#endif //CHATSERVER_CHATSERVICE_H
