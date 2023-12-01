#if 0

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