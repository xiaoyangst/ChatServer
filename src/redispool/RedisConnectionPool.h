/**
  ******************************************************************************
  * @file           : RedisConnectionPool.h
  * @author         : xy
  * @brief          : None
  * @attention      : None
  * @date           : 2025/4/17
  ******************************************************************************
  */

#ifndef CHAT_SRC_REDISPOOL_REDISCONNECTIONPOOL_H_
#define CHAT_SRC_REDISPOOL_REDISCONNECTIONPOOL_H_

#include "RedisConn.h"
#include <queue>
#include <list>
#include <mutex>
#include <condition_variable>
#include <memory>

class RedisConnectionPool {
 public:
  static RedisConnectionPool* getInstance();
  std::shared_ptr<RedisConn> getConn();

  void destroyPool();	// 用户可以手动删除所有连接，也可以选择在 析构中去自动调用

  RedisConnectionPool();
  ~RedisConnectionPool();

  void init(const redis_message_handler &handler);

  RedisConnectionPool(const RedisConnectionPool &) = delete;
  RedisConnectionPool &operator=(const RedisConnectionPool &) = delete;

  void releaseConn(RedisConn *conn);
 private:
  std::list<RedisConn*> pool_;
  std::mutex mtx_;
  std::condition_variable cond_;
};

#endif //CHAT_SRC_REDISPOOL_REDISCONNECTIONPOOL_H_
