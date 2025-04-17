/**
  ******************************************************************************
  * @file           : RedisConnectionPool.cpp
  * @author         : xy
  * @brief          : None
  * @attention      : None
  * @date           : 2025/4/17
  ******************************************************************************
  */

#include "RedisConnectionPool.h"

const int max_conn = 4;
const std::string host_ = "127.0.0.1";
const int port_ = 6379;

RedisConnectionPool::~RedisConnectionPool() {
	destroyPool();
}

RedisConnectionPool *RedisConnectionPool::getInstance() {
	static RedisConnectionPool pool;
	return &pool;
}

RedisConnectionPool::RedisConnectionPool() {
	std::lock_guard<std::mutex> lg(mtx_);    // 避免还在初始化的时候，就被人去获取连接
	for (int i = 0; i < max_conn; ++i) {
		auto *conn = new RedisConn(host_, port_);
		if (conn->connect()) {
			pool_.push_back(conn);
		} else {
			delete conn;
		}
	}
}

void RedisConnectionPool::init(const redis_message_handler &handler) {
	std::lock_guard<std::mutex> lg(mtx_);
	for (const auto &kItem : pool_) {
		kItem->init_notify_handler(handler);
	}
}

std::shared_ptr<RedisConn> RedisConnectionPool::getConn() {
	std::lock_guard<std::mutex> lg(mtx_);
	auto conn = pool_.front();
	pool_.pop_front();
	auto *pool = RedisConnectionPool::getInstance();
	return std::shared_ptr<RedisConn>(conn, [pool](RedisConn *redis_conn) {
	  pool->releaseConn(redis_conn);    // 自定义删除器，本质是回收连接到池中
	});
}

void RedisConnectionPool::releaseConn(RedisConn *conn) {
	std::lock_guard<std::mutex> lg(mtx_);
	pool_.push_back(conn);
	cond_.notify_one();
}

void RedisConnectionPool::destroyPool() {
	std::lock_guard<std::mutex> lg(mtx_);
	while (!pool_.empty()) {
		RedisConn *conn = pool_.front();
		pool_.pop_front();
		conn->disconnect();
		delete conn;
	}
}

