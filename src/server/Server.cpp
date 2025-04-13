#include <iostream>
#include "ChatServer.h"
#include "ChatService.h"
#include "net/Logger.h"
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

	EventLoop loop;
//    InetAddress addr("127.0.0.1", 8080);
	std::string ip = "127.0.0.1";
	InetAddress addr(atoi(argv[1]),ip);
	ChatServer server(&loop, addr, "ChatServer");

	server.start();
	loop.loop();

	return 0;
}

//注册业务测试    {"msgid":4,"name":"xyasu","pwd":"123456"}

//登录业务测试    {"msgid":1,"id":21,"pwd":"123456"}

//客户端异常退出测试     ps查看客户端pid，然后通过kill命令杀死进程模拟客户端异常退出

//点对点聊天测试
/*
 * {"msgid":1,"id":18,"pwd":"123456"}
 * {"msgid":1,"id":19,"pwd":"123456"}
 * {"from":"wu yang","id":18,"msgid":6,"toid":19,"msg":"hello,pi pi"}
 * {"from":"pi pi","id":19,"msgid":6,"toid":18,"msg":"hello,wu yang"}
 */

//离线消息测试
/*
 * {"msgid":1,"id":18,"pwd":"123456"}
 * {"msgid":1,"id":19,"pwd":"123456"}
 * {"from":"wu yang","id":18,"msgid":6,"toid":21,"msg":"hello,gao yang"}
 * {"from":"pi pi","id":19,"msgid":6,"toid":21,"msg":"hello,gao yang"}
 * {"msgid":1,"id":21,"pwd":"123456"}
 */

//添加好友测试
/*
 * {"msgid":1,"id":18,"pwd":"123456"}
 * {"msgid":7,"id":18,"friendid":23}
 * {"msgid":7,"id":18,"friendid":16}
 */

//群组相关业务测试
////创建群组测试
/*
 *{"msgid":8,"id":18,"groupname":"Java group","groupdesc":"study java"}
 */
////加入群组测试
/*
 * {"msgid":9,"id":18,"groupid":1}
 */
////群聊测试
/*
 * {"msgid":10,"id":18,"groupid":1,"msg":"hello"}
 * {"msgid":1,"id":18,"pwd":"123456"}
 */