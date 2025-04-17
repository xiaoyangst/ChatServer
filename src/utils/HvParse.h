/**
  ******************************************************************************
  * @file           : HvParse.h
  * @author         : xy
  * @brief          : None
  * @attention      : None
  * @date           : 2025/4/17
  ******************************************************************************
  */

#ifndef CHAT_SRC_UTILS_HVPARSE_H_
#define CHAT_SRC_UTILS_HVPARSE_H_

#include <string>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
constexpr size_t SERVER_HEAD_LENGTH = 4;
constexpr size_t SERVER_HEAD_LENGTH_FIELD_OFFSET = 0;
constexpr size_t SERVER_HEAD_LENGTH_FIELD_BYTES = 4;

class HvParse {
 public:
  // 封包函数，将字符串封装成自定义协议格式（头部+数据）
  static std::string packMessageAsString(const std::string &message);
  // 拆包函数，从接收到的数据中提取消息
  static std::string unpackMessage(const std::string &receivedData);
};

#endif //CHAT_SRC_UTILS_HVPARSE_H_
