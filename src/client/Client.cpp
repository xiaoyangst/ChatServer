#include "json.hpp"
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <chrono>
#include <functional>
#include <ctime>
#include <unordered_map>
using namespace std;
using json = nlohmann::json;

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <atomic>
#include "hv/TcpClient.h"
#include "utils/HvParse.h"
#include "Group.h"
#include "User.h"
#include "public.h"

using namespace hv;

// 记录当前系统登录的用户信息
User g_currentUser;
// 记录当前登录用户的好友列表信息
vector<User> g_currentUserFriendList;
// 记录当前登录用户的群组列表信息
vector<Group> g_currentUserGroupList;

// 控制主菜单页面程序
bool isMainMenuRunning = false;

// 用于读写线程之间的通信
sem_t rwsem;
// 记录登录状态
atomic_bool g_isLoginSuccess{false};

void doRegResponse(json &responsejs);
void doLoginResponse(json &responsejs);
// 获取系统时间（聊天信息需要添加时间信息）
string getCurrentTime();
// 主聊天页面程序
void mainMenu(const hv::SocketChannelPtr &channel);
// 显示当前登录成功用户的基本信息
void showCurrentUserData();

void backMenu(const hv::SocketChannelPtr &channel, json &js);
void frontMenu(const hv::SocketChannelPtr &channel, const json &jsh = json::object());

hv::SocketChannelPtr mainChannel = nullptr;

// 聊天客户端程序实现，main线程用作发送线程，子线程用作接收线程
int main(int argc, char **argv) {
	if (argc < 3) {
		cerr << "command invalid! example: ./ChatClient 127.0.0.1 6000" << endl;
		exit(-1);
	}

	// 解析通过命令行参数传递的ip和port
	char *ip = argv[1];
	uint16_t port = atoi(argv[2]);

	hv::TcpClient tcp_client;
	int m_connfd = tcp_client.createsocket(port, ip);
	if (m_connfd < 0) {
		printf("connfd failed\n");
		return -1;
	}
	auto *server_unpack_setting = new unpack_setting_t();
	memset(server_unpack_setting, 0, sizeof(unpack_setting_t));
	server_unpack_setting->mode = UNPACK_BY_LENGTH_FIELD;
	server_unpack_setting->package_max_length = DEFAULT_PACKAGE_MAX_LENGTH;
	server_unpack_setting->body_offset = SERVER_HEAD_LENGTH;
	server_unpack_setting->length_field_offset = SERVER_HEAD_LENGTH_FIELD_OFFSET;
	server_unpack_setting->length_field_bytes = SERVER_HEAD_LENGTH_FIELD_BYTES;
	server_unpack_setting->length_field_coding = ENCODE_BY_BIG_ENDIAN;
	tcp_client.setUnpack(server_unpack_setting);

	// 连接建立回调
	tcp_client.onConnection = [&](const hv::SocketChannelPtr &channel) {
	  std::string peeraddr = channel->peeraddr();
	  if (channel->isConnected()) {
		  printf("onConnection connected to %s! connfd=%d\n", peeraddr.c_str(), channel->fd());
		  mainChannel = channel;
		  frontMenu(channel);
	  } else {
		  printf("onConnection disconnected to %s! connfd=%d\n", peeraddr.c_str(), channel->fd());
		  printf("连接断开\n");
		  return;
	  }
	};

	// 业务回调
	tcp_client.onMessage = [&](const hv::SocketChannelPtr &channel, hv::Buffer *buf) {
	  std::cout << "onMessage" << std::endl;
	  std::string peeraddr = channel->peeraddr();
	  std::string data = std::string((char *)buf->data(), buf->size());
	  auto message = HvParse::packMessageAsString(data);
	  json js = json::parse(message);
	  backMenu(channel, js);
	};

	tcp_client.start();

	// 防止主线程退出
	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // 防止主线程退出
	}

	return 0;
}

// 处理注册的响应逻辑
void doRegResponse(json &responsejs) {
	if (0 != responsejs["errno"].get<int>()) // 注册失败
	{
		cerr << "name is already exist, register error!" << endl;
	} else // 注册成功
	{
		cout << "name register success, userid is " << responsejs["id"]
			 << ", do not forget it!" << endl;
	}
}

// 处理登录的响应逻辑
void doLoginResponse(json &responsejs) {
	if (0 != responsejs["errno"].get<int>()) // 登录失败
	{
		cerr << responsejs["errmsg"] << endl;
		g_isLoginSuccess = false;
	} else // 登录成功
	{
		// 记录当前用户的id和name
		g_currentUser.setId(responsejs["id"].get<int>());
		g_currentUser.setName(responsejs["name"]);

		// 记录当前用户的好友列表信息
		if (responsejs.contains("friends")) {
			// 初始化
			g_currentUserFriendList.clear();

			vector<string> vec = responsejs["friends"];
			for (string &str : vec) {
				json js = json::parse(str);
				User user;
				user.setId(js["id"].get<int>());
				user.setName(js["name"]);
				user.setState(js["state"]);
				g_currentUserFriendList.push_back(user);
			}
		}

		// 记录当前用户的群组列表信息
		if (responsejs.contains("groups")) {
			// 初始化
			g_currentUserGroupList.clear();

			vector<string> vec1 = responsejs["groups"];
			for (string &groupstr : vec1) {
				json grpjs = json::parse(groupstr);
				Group group;
				group.setId(grpjs["id"].get<int>());
				group.setName(grpjs["groupname"]);
				group.setDesc(grpjs["groupdesc"]);

				vector<string> vec2 = grpjs["users"];
				for (string &userstr : vec2) {
					GroupUser user;
					json js = json::parse(userstr);
					user.setId(js["id"].get<int>());
					user.setName(js["name"]);
					user.setState(js["state"]);
					user.setRole(js["role"]);
					group.getUsers().push_back(user);
				}

				g_currentUserGroupList.push_back(group);
			}
		}

		// 显示登录用户的基本信息
		showCurrentUserData();

		// 显示当前用户的离线消息  个人聊天信息或者群组消息
		if (responsejs.contains("offlinemsg")) {
			vector<string> vec = responsejs["offlinemsg"];
			for (string &str : vec) {
				json js = json::parse(str);
				if (ONE_CHAT_MSG == js["msgid"].get<int>()) {
					cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
						 << " said: " << js["msg"].get<string>() << endl;
				} else {
					cout << "群消息[" << js["groupid"] << "]:" << js["time"].get<string>() << " [" << js["id"] << "]"
						 << js["name"].get<string>()
						 << " said: " << js["msg"].get<string>() << endl;
				}
			}
		}

		g_isLoginSuccess = true;
	}
}

// 显示当前登录成功用户的基本信息
void showCurrentUserData() {
	cout << "======================login user======================" << endl;
	cout << "current login user => id:" << g_currentUser.getId() << " name:" << g_currentUser.getName() << endl;
	cout << "----------------------friend list---------------------" << endl;
	if (!g_currentUserFriendList.empty()) {
		for (User &user : g_currentUserFriendList) {
			cout << user.getId() << " " << user.getName() << " " << user.getState() << endl;
		}
	}
	cout << "----------------------group list----------------------" << endl;
	if (!g_currentUserGroupList.empty()) {
		for (Group &group : g_currentUserGroupList) {
			cout << group.getId() << " " << group.getName() << " " << group.getDesc() << endl;
			for (GroupUser &user : group.getUsers()) {
				cout << user.getId() << " " << user.getName() << " " << user.getState()
					 << " " << user.getRole() << endl;
			}
		}
	}
	cout << "======================================================" << endl;
}

// "help" command handler
void help(const hv::SocketChannelPtr &channel = nullptr, const string &str = "");
// "chat" command handler
void chat(const hv::SocketChannelPtr &channel, const string &);
// "addfriend" command handler
void addfriend(const hv::SocketChannelPtr &channel, const string &);
// "creategroup" command handler
void creategroup(const hv::SocketChannelPtr &channel, const string &);
// "addgroup" command handler
void addgroup(const hv::SocketChannelPtr &channel, const string &);
// "groupchat" command handler
void groupchat(const hv::SocketChannelPtr &channel, const string &);
// "loginout" command handler
void loginout(const hv::SocketChannelPtr &channel, const string &);

// 系统支持的客户端命令列表
unordered_map<string, string> commandMap = {
	{"help", "显示所有支持的命令，格式: help"},
	{"chat", "一对一聊天，格式: chat:friendid:message"},
	{"addfriend", "添加好友，格式: addfriend:friendid"},
	{"creategroup", "创建群组，格式: creategroup:groupname:groupdesc"},
	{"addgroup", "加入群组，格式: addgroup:groupid"},
	{"groupchat", "群聊，格式: groupchat:groupid:message"},
	{"loginout", "注销，格式: loginout"}
};

// 注册系统支持的客户端命令处理
unordered_map<string, std::function<void(const hv::SocketChannelPtr &, string)>> commandHandlerMap = {
	{"help", help},
	{"chat", chat},
	{"addfriend", addfriend},
	{"creategroup", creategroup},
	{"addgroup", addgroup},
	{"groupchat", groupchat},
	{"loginout", loginout}
};

void frontMenu(const hv::SocketChannelPtr &channel, const json &jsh) {
	// 显示首页面菜单 登录、注册、退出
	cout << "========================" << endl;
	cout << "1. login" << endl;
	cout << "2. register" << endl;
	cout << "3. quit" << endl;
	cout << "========================" << endl;
	cout << "choice:";
	int choice = 0;
	cin >> choice;
	cin.get(); // 读掉缓冲区残留的回车
	switch (choice) {
		case 1: // login业务
		{
			int id = 0;
			char pwd[50] = {0};
			cout << "id:";
			cin >> id;
			cin.get(); // 读掉缓冲区残留的回车
			cout << "password:";
			cin.getline(pwd, 50);

			json js;
			js["msgid"] = LOGIN_MSG;
			js["id"] = id;
			js["pwd"] = pwd;
			string request = js.dump();
			string message = HvParse::packMessageAsString(request);

			channel->write(message);
			isMainMenuRunning = true;
			mainMenu(channel);
		}
			break;
		case 2: // register业务
		{
			char name[50] = {0};
			char pwd[50] = {0};
			cout << "username:";
			cin.getline(name, 50);
			cout << "password:";
			cin.getline(pwd, 50);

			json js;
			js["msgid"] = REGISTER_MSG;
			js["name"] = name;
			js["pwd"] = pwd;
			string request = js.dump();
			string message = HvParse::packMessageAsString(request);

			channel->write(message);
			frontMenu(channel);
		}
			break;
		case 3: // quit业务
			channel->close();
			exit(0);
		default:cerr << "invalid input!" << endl;
			break;
	}
}

void backMenu(const hv::SocketChannelPtr &channel, json &js) {
	auto msgtype = js["msgid"].get<int>();
	if (ONE_CHAT_MSG == msgtype) {
		cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
			 << " said: " << js["msg"].get<string>() << endl;
	} else if (GROUP_CHAT_MSG == msgtype) {
		cout << "群消息[" << js["groupid"] << "]:" << js["time"].get<string>() << " [" << js["id"] << "]"
			 << js["name"].get<string>()
			 << " said: " << js["msg"].get<string>() << endl;
	} else if (LOGIN_MSG_ACK == msgtype) {
		doLoginResponse(js); // 处理登录响应的业务逻辑
		isMainMenuRunning = true;
		mainMenu(channel);
	} else if (REGISTER_MSG_ACK == msgtype) {
		cout << "register msg ack" << endl;
		doRegResponse(js);
		frontMenu(channel);
	}
}

// 主聊天页面程序
void mainMenu(const hv::SocketChannelPtr &channel) {
	help();

	char buffer[1024] = {0};
	while (isMainMenuRunning) {
		cin.getline(buffer, 1024);
		string commandbuf(buffer);
		string command; // 存储命令
		int idx = commandbuf.find(":");
		if (-1 == idx) {
			command = commandbuf;
		} else {
			command = commandbuf.substr(0, idx);
		}
		auto it = commandHandlerMap.find(command);
		if (it == commandHandlerMap.end()) {
			cerr << "invalid input command!" << endl;
			continue;
		}

		// 调用相应命令的事件处理回调，mainMenu对修改封闭，添加新功能不需要修改该函数
		it->second(channel, commandbuf.substr(idx + 1, commandbuf.size() - idx)); // 调用命令处理方法
	}
}

// "help" command handler
void help(const hv::SocketChannelPtr &channel, const string &str) {
	cout << "show command list >>> " << endl;
	for (auto &p : commandMap) {
		cout << p.first << " : " << p.second << endl;
	}
	cout << endl;
}

// "addfriend" command handler
void addfriend(const hv::SocketChannelPtr &channel, const string &str) {
	int friendid = atoi(str.c_str());
	json js;
	js["msgid"] = ADD_FRIEND_MSG;
	js["id"] = g_currentUser.getId();
	js["friendid"] = friendid;
	string buffer = js.dump();
	string data = HvParse::packMessageAsString(buffer);
	channel->write(data);
	mainMenu(channel);
}

// "chat" command handler
void chat(const hv::SocketChannelPtr &channel, const string &str) {
	cout << str << endl;
	int idx = str.find(":"); // friendid:message
	if (-1 == idx) {
		cerr << "chat command invalid!" << endl;
		return;
	}

	int friendid = atoi(str.substr(0, idx).c_str());
	string message = str.substr(idx + 1, str.size() - idx);

	json js;
	js["msgid"] = ONE_CHAT_MSG;
	js["id"] = g_currentUser.getId();
	js["name"] = g_currentUser.getName();
	js["toid"] = friendid;
	js["msg"] = message;
	js["time"] = getCurrentTime();
	string buffer = js.dump();
	string data = HvParse::packMessageAsString(buffer);
	channel->write(data);
	mainMenu(channel);
}

// "creategroup" command handler  groupname:groupdesc
void creategroup(const hv::SocketChannelPtr &channel, const string &str) {
	int idx = str.find(":");
	if (-1 == idx) {
		cerr << "creategroup command invalid!" << endl;
		return;
	}

	string groupname = str.substr(0, idx);
	string groupdesc = str.substr(idx + 1, str.size() - idx);

	json js;
	js["msgid"] = CREATE_GROUP_MSG;
	js["id"] = g_currentUser.getId();
	js["groupname"] = groupname;
	js["groupdesc"] = groupdesc;

	string buffer = js.dump();
	string data = HvParse::packMessageAsString(buffer);
	channel->write(data);
	mainMenu(channel);
}

// "addgroup" command handler
void addgroup(const hv::SocketChannelPtr &channel, const string &str) {
	int groupid = atoi(str.c_str());
	json js;
	js["msgid"] = ADD_GROUP_MSG;
	js["id"] = g_currentUser.getId();
	js["groupid"] = groupid;
	string buffer = js.dump();
	string data = HvParse::packMessageAsString(buffer);
	channel->write(data);
	mainMenu(channel);
}

// "groupchat" command handler   groupid:message
void groupchat(const hv::SocketChannelPtr &channel, const string &str) {
	int idx = str.find(":");
	if (-1 == idx) {
		cerr << "groupchat command invalid!" << endl;
		return;
	}

	int groupid = atoi(str.substr(0, idx).c_str());
	string message = str.substr(idx + 1, str.size() - idx);

	json js;
	js["msgid"] = GROUP_CHAT_MSG;
	js["id"] = g_currentUser.getId();
	js["name"] = g_currentUser.getName();
	js["groupid"] = groupid;
	js["msg"] = message;
	js["time"] = getCurrentTime();
	string buffer = js.dump();
	string data = HvParse::packMessageAsString(buffer);
	channel->write(data);
	mainMenu(channel);
}

// "loginout" command handler
void loginout(const hv::SocketChannelPtr &channel, const string &) {
	json js;
	js["msgid"] = LOGINOUT_MSG;
	js["id"] = g_currentUser.getId();
	string buffer = js.dump();
	string data = HvParse::packMessageAsString(buffer);
	channel->write(data);
	isMainMenuRunning = false;
}

// 获取系统时间（聊天信息需要添加时间信息）
string getCurrentTime() {
	auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	struct tm *ptm = localtime(&tt);
	char date[60] = {0};
	sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
			(int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
			(int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
	return std::string(date);
}
