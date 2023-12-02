#if 1

#include <iostream>
#include "ChatServer.h"
#include "ChatService.h"
#include <muduo/base/Logging.h>

int main(int argc, char **argv){
    EventLoop loop;
    InetAddress addr("127.0.0.1", 8080);
    ChatServer server(&loop, addr, "ChatServer");

    server.start();
    loop.loop();

    return 0;
}

#endif

//注册业务测试    {"msgid":4,"name":"xy_user","pwd":"123456"}

//登录业务测试    {"msgid":1,"id":24,"pwd":"123456"}