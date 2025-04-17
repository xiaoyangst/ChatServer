/**
  ******************************************************************************
  * @file           : nocopyable.h
  * @author         : xy
  * @brief          : None
  * @attention      : None
  * @date           : 2025/4/17
  ******************************************************************************
  */

#ifndef CHAT_SRC_UTILS_NOCOPYABLE_H_
#define CHAT_SRC_UTILS_NOCOPYABLE_H_

class noncopyable {
 protected:
  noncopyable() = default;
  ~noncopyable() = default;
  // 删除拷贝构造和拷贝赋值
  noncopyable(const noncopyable &) = delete;
  noncopyable &operator=(const noncopyable &) = delete;
};


#endif //CHAT_SRC_UTILS_NOCOPYABLE_H_
