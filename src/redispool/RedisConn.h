/**
  ******************************************************************************
  * @file           : RedisConn.h
  * @author         : xy
  * @brief          : None
  * @attention      : None
  * @date           : 2025/4/17
  ******************************************************************************
  */

#ifndef CHAT_SRC_REDISPOOL_REDISCONN_H_
#define CHAT_SRC_REDISPOOL_REDISCONN_H_

#include <hiredis/hiredis.h>
#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>

using redis_message_handler = std::function<void(std::string, std::string)>;

class RedisConn {
 public:
  RedisConn(std::string host, int port);
  ~RedisConn();
  bool connect();
  void disconnect();

  // 基本操作
  bool set(const std::string &key, const std::string &value);
  std::string get(const std::string &key);
  bool del(const std::string &key);

  // 发布-订阅模式
  bool publish(const std::string &channel, const std::string &message);
  bool subscribe(const std::string &channel);
  bool unsubscribe(const std::string &channel);

  //独立线程中接收订阅通道的消息
  void observer_channel_message();

  //初始化业务层上报通道消息的回调对象
  void init_notify_handler(redis_message_handler handler);
 private:
  std::string host_;
  int port_;
  redisContext *context;
  redisContext *publishContext;    // 用于发布
  redisContext *subContext; // 用于订阅
  std::thread subThread;
  std::function<void(std::string, std::string)> notify_message_handler_;
};

#endif //CHAT_SRC_REDISPOOL_REDISCONN_H_


/*异步订阅
 * 第一个参数是发布者的唯一标识
 * 第二个参数是给业务层提供的，允许这边订阅成功之后，给业务层一个反馈/提示
    client.subscribe_once("test-channel", [](const std::string& channel, const std::string& message) {
        std::cout << "[SUB] [" << channel << "] " << message << std::endl;
    });
 */