/**
  ******************************************************************************
  * @file           : RedisConn.cpp
  * @author         : xy
  * @brief          : None
  * @attention      : None
  * @date           : 2025/4/17
  ******************************************************************************
  */

#include "RedisConn.h"
#include <iostream>
#include <utility>

RedisConn::RedisConn(std::string host, int port)
	: host_(std::move(host)), port_(port)
	  , context(nullptr)
	  , publishContext(nullptr)
	  , subContext(nullptr) {}

RedisConn::~RedisConn() {
	disconnect();
}

bool RedisConn::connect() {
	context = redisConnect(host_.c_str(), port_);
	if (!context || context->err) {
		std::cerr << "context Connection error: " << (context ? context->errstr : "null context") << std::endl;
		return false;
	}
	publishContext = redisConnect(host_.c_str(), port_);
	if (!publishContext || publishContext->err) {
		std::cerr << "publishContext Connection error: "
				  << (publishContext ? publishContext->errstr : "null publishContext") << std::endl;
		return false;
	}

	subContext = redisConnect(host_.c_str(), port_);
	if (!subContext || subContext->err) {
		std::cerr << "subContext Connection error: " << (subContext ? subContext->errstr : "null subContext")
				  << std::endl;
		return false;
	}

	subThread = std::thread(&RedisConn::observer_channel_message, this);    // 要不接收发布的消息
	subThread.detach();

	std::cout << "Redis success" << std::endl;

	return true;
}

void RedisConn::disconnect() {
	if (context) {
		redisFree(context);
		context = nullptr;
	}

	if (publishContext) {
		redisFree(publishContext);
		publishContext = nullptr;
	}

	if (subContext) {
		redisFree(subContext);
		subContext = nullptr;
	}
}

bool RedisConn::set(const std::string &key, const std::string &value) {
	auto *reply = (redisReply *)redisCommand(context, "SET %s %s", key.c_str(), value.c_str());
	bool success = reply && reply->type == REDIS_REPLY_STATUS && std::string(reply->str) == "OK";
	freeReplyObject(reply);
	return success;
}

std::string RedisConn::get(const std::string &key) {
	auto *reply = (redisReply *)redisCommand(context, "GET %s", key.c_str());
	std::string result;
	if (reply && reply->type == REDIS_REPLY_STRING) {
		result = reply->str;
	}
	freeReplyObject(reply);
	return result;
}

bool RedisConn::del(const std::string &key) {
	auto *reply = (redisReply *)redisCommand(context, "DEL %s", key.c_str());
	bool success = reply && reply->integer > 0;
	freeReplyObject(reply);
	return success;
}

bool RedisConn::publish(const std::string &channel, const std::string &message) {
	auto *reply = (redisReply *)redisCommand(publishContext, "PUBLISH %s %s", channel.c_str(), message.c_str());
	if (reply == nullptr) {
		std::cerr << "publish command failed!" << std::endl;
		return false;
	}

	// 释放资源
	freeReplyObject(reply);
	return true;
}

bool RedisConn::subscribe(const std::string &channel) {

	if (REDIS_ERR == redisAppendCommand(subContext, "SUBSCRIBE %s", channel.c_str())) {
		std::cerr << "subscibe command failed" << std::endl;
		return false;
	}

	int done = 0;
	while (!done) {    // 不是死循环，因为 redisBufferWrite 会在发送完所有缓冲命令后把 done 设置为 1，从而跳出循环。
		if (REDIS_ERR == redisBufferWrite(subContext, &done)) {
			std::cerr << "subscribe command failed" << std::endl;
			return false;
		}
	}

	std::cout << channel << "subscribe" << std::endl;

	return true;
}

bool RedisConn::unsubscribe(const std::string &channel) {
	if (REDIS_ERR == redisAppendCommand(subContext, "SUBSCRIBE %s", channel.c_str())) {
		std::cerr << "subscibe command failed" << std::endl;
		return false;
	}

	int done = 0;
	while (!done) {
		if (REDIS_ERR == redisBufferWrite(subContext, &done)) {
			std::cerr << "subscribe command failed" << std::endl;
			return false;
		}
	}

	return true;
}

void RedisConn::observer_channel_message() {
	redisReply *reply = nullptr;
	while (REDIS_OK == redisGetReply(subContext, (void **)&reply)) {
		//reply里面是返回的数据有三个，0. message , 1.通道号，2.消息
		if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr) {
			//给业务层上报消息
			notify_message_handler_(reply->element[1]->str, reply->element[2]->str);
		}

		freeReplyObject(reply);
	}
}

void RedisConn::init_notify_handler(redis_message_handler handler) {
	notify_message_handler_ = std::move(handler);
}
