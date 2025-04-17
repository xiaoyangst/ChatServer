#include <iostream>
#include "ChatServer.h"
#include "ChatService.h"
#include "utils/Log.h"
#include <csignal>

// 捕获SIGINT的处理函数
void resetHandler(int) {
	LOG_INFO("capture the SIGINT, will reset state\n");
	ChatService::instance()->reset();
	exit(0);
}

int main(int argc, char **argv) {

	// 向SIGINT信号注册resetHandler回调函数
	signal(SIGINT, resetHandler);

	std::string ip = "127.0.0.1";

	ChatServer server(ip,atoi(argv[1]),4);

	server.start();

	return 0;
}
