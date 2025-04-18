/**
  ******************************************************************************
  * @file           : HvParse.cpp
  * @author         : xy
  * @brief          : None
  * @attention      : None
  * @date           : 2025/4/17
  ******************************************************************************
  */

#include "HvParse.h"

std::string HvParse::packMessageAsString(const std::string &message) {
	// 自定义协议格式：头部长度（4字节）+数据
	std::string packedMessage;
	packedMessage.resize(SERVER_HEAD_LENGTH + message.length());
	auto length = htonl(message.size());
	std::memcpy(packedMessage.data(), &length, SERVER_HEAD_LENGTH);
	std::memcpy(packedMessage.data() + SERVER_HEAD_LENGTH, message.data(), message.length());
	return packedMessage;
}
std::string HvParse::unpackMessage(const std::string &receivedData) {
	// 自定义协议格式：头部长度（4字节）+数据
	std::string unpackedMessage;
	auto length = receivedData.size() - SERVER_HEAD_LENGTH;
	unpackedMessage.resize(length);
	std::memcpy(unpackedMessage.data(), receivedData.data() + SERVER_HEAD_LENGTH, length);
	return unpackedMessage;
}