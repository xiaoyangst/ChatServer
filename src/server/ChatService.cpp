#include "ChatService.h"

#include "public.h"
#include "Group.h"
#include <iostream>
#include <net/Logger.h>

using namespace placeholders;

//注册消息以及对应的Handle对应的回调函数
ChatService::ChatService() {
	_msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::loginHandler, this, _1, _2, _3)});
	_msgHandlerMap.insert({REGISTER_MSG, std::bind(&ChatService::registerHandler, this, _1, _2, _3)});
	_msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChatHandler, this, _1, _2, _3)});
	_msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriendHandler, this, _1, _2, _3)});
	_msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
	_msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
	_msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::chatGroup, this, _1, _2, _3)});
	_msgHandlerMap.insert({LOGINOUT_MSG, std::bind(&ChatService::clientLogout, this, _1, _2, _3)});

	if (_redis.connect()) {
		_redis.init_notify_handler(std::bind(&ChatService::redis_subscribe_message_handler, this, _1, _2));
	}
}

MsgHandler ChatService::getHandler(int msgId) {
	auto it = _msgHandlerMap.find(msgId);
	if (it != _msgHandlerMap.end()) {
		return _msgHandlerMap[msgId];
	} else {
		//返回值本质上是一个函数，因此这里我们就用lambad帮我们实现并返回，只是简单的提示
		return [=](const TcpConnectionPtr &conn, json &js, Timestamp) {
		  LOG_ERROR ("msgId: %d can not find handler!", msgId);
		};
	}
}

//用户提供账号和密码
void ChatService::registerHandler(const TcpConnectionPtr &conn, json &js, Timestamp time) {
	LOG_DEBUG ("start do register !");

	std::string name = js["name"];
	std::string pwd = js["pwd"];

	User user;
	user.setName(name);
	user.setPassword(pwd);

	bool state = _userModel.insert(user);   //记录插入是否成功的状态保存到state变量中，true   /   false

	if (state) { //注册成功

		LOG_INFO("%s:register success!", name.c_str());

		json response;
		response["msgid"] = REGISTER_MSG_ACK;
		response["errno"] = 0;
		response["id"] = user.getId();

		//dump方法把json转换为std::string
		conn->send(response.dump());
		cout << "register info to client" << endl;
	} else {
		json response;
		response["msgid"] = REGISTER_MSG_ACK;
		response["errno"] = 1;
		// 注册已经失败，不需要在json返回id
		conn->send(response.dump());
	}

}

void ChatService::loginHandler(const TcpConnectionPtr &conn, json &js, Timestamp time) {
	LOG_DEBUG("start do login !");

	int id = js["id"].get<int>();
	std::string pwd = js["pwd"];

	User user = _userModel.qurry(id);
	if (user.getId() == id && user.getPassword() == pwd) {
		if (user.getState() == "online") {
			//不可重复登录
			json response;
			response["msgid"] = LOGIN_MSG_ACK;
			response["errno"] = 2;
			response["errmsg"] = "this account is using, input another!";
			conn->send(response.dump());
		} else { //登录成功

			LOG_INFO("%s:login success!", user.getName().c_str());

			{
				lock_guard<mutex> lockGuard(_conmutex);
				_userConnMap.insert({id, conn});
			}

			// id用户登录成功后，向redis订阅channel(id)
			_redis.subscribe(id);

			user.setState("online");
			_userModel.updateState(user);

			json response;
			response["msgid"] = LOGIN_MSG_ACK;
			response["errno"] = 0;
			response["id"] = user.getId();
			response["name"] = user.getName();

			//查看是否有离线消息
			std::vector<std::string> result = _offlineMsgModel.query(id);
			if (!result.empty()) {
				//有离线消息
				response["offlinemsg"] = result;
				//读取之后就该移除离线消息
				_offlineMsgModel.remove(id);
			} else {
				LOG_INFO("no offline msg");
			}

			//显示好友列表
			std::vector<User> userRes = _friendModel.query(id);
			if (!userRes.empty()) {
				std::vector<std::string> vec;
				for (auto &userres : userRes) {
					json jsuser;
					jsuser["id"] = user.getId();
					jsuser["name"] = user.getName();
					jsuser["state"] = user.getState();
					vec.push_back(js.dump());
				}
				response["fiends"] = vec;
			}

			//显示群组列表
			std::vector<Group> groupres = _groupModel.queryGroups(id);
			if (!groupres.empty()) {
				std::vector<std::string> vec;
				for (auto &group : groupres) {
					json jsgroup;
					jsgroup["id"] = group.getId();
					jsgroup["name"] = group.getName();
					jsgroup["desc"] = group.getDesc();
					vec.push_back(jsgroup.dump());
				}
				response["group"] = vec;
			}

			conn->send(response.dump());
		}
	} else {  //登录失败
		json response;
		response["msgid"] = LOGIN_MSG_ACK;
		response["errno"] = -1;
		response["errmsg"] = "login failed ,Try again!\n";
		conn->send(response.dump());
	}
}

void ChatService::clientCloseException(const TcpConnectionPtr &conn) {
	User user;
	{   //找到连接，并在 _userConnMap 删除它
		lock_guard<mutex> lockGuard(_conmutex);
		for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it) {
			if (it->second == conn) {
				user.setId(it->first);
				_userConnMap.erase(it);
				break;
			}
		}
	}

	// 用户注销
	_redis.unsubscribe(user.getId());

	//更新用户状态
	if (user.getId() != -1) {
		user.setState("offline");
		_userModel.updateState(user);
	}
}

//如果用户登录状态，直接给他发消息
//如果用户离线状态，把离线消息存储起来，等他上线时候读取
void ChatService::oneChatHandler(const TcpConnectionPtr &conn, json &js, Timestamp time) {
	//先获取 聊天对象的ID
	int toId = js["toid"].get<int>();
	cout << "Server Buffer " << js.dump() << endl;
	{
		lock_guard<mutex> lockGuard(_conmutex);
		auto it = _userConnMap.find(toId);
		if (it != _userConnMap.end()) {  //确认在线
			it->second->send( js.dump());
			return;
		}
	}

	// 用户在其他主机的情况，publish消息到redis
	User user = _userModel.qurry(toId);
	if (user.getState() == "online") {
		_redis.publish(toId, js.dump());
		return;
	}

	//处理离线
	_offlineMsgModel.insert(toId, js.dump());
}

void ChatService::reset() {
	//全部online转换为offline
	_userModel.resetState();
}

void ChatService::addFriendHandler(const TcpConnectionPtr &conn, json &js, Timestamp time) {
	int userId = js["id"].get<int>();
	int friendId = js["friendid"].get<int>();

	_friendModel.insert(userId, friendId);
}

void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time) {
	int userId = js["id"].get<int>();
	std::string name = js["groupname"];
	std::string desc = js["groupdesc"];


	// 存储新创建的群组消息
	Group group(-1, name, desc);
	if (_groupModel.createGroup(group)) {
		// 存储群组创建人信息
		_groupModel.addGroup(userId, group.getId(), "creator");
	}
}

void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time) {
	int userId = js["id"].get<int>();
	int groupId = js["groupid"].get<int>();
	_groupModel.addGroup(userId, groupId, "normal");
}

void ChatService::chatGroup(const TcpConnectionPtr &conn, json &js, Timestamp time) {
	int userId = js["id"].get<int>();
	int groupId = js["groupid"].get<int>();
	std::vector<int> userIdVec = _groupModel.queryGroupUsers(userId, groupId);

	lock_guard<mutex> lock(_conmutex);
	for (int id : userIdVec) {
		auto it = _userConnMap.find(id);
		if (it != _userConnMap.end()) {
			it->second->send(js.dump());
		} else {

			User user = _userModel.qurry(id);
			if (user.getState() == "online") {
				_redis.publish(id, js.dump());
			} else {
				_offlineMsgModel.insert(id, js.dump());
			}
		}
	}
}

// redis订阅消息触发的回调函数,这里channel其实就是id
void ChatService::redis_subscribe_message_handler(int channel, string message) {
	//用户在线
	lock_guard<mutex> lock(_conmutex);
	auto it = _userConnMap.find(channel);
	if (it != _userConnMap.end()) {
		it->second->send(message);
		return;
	}

	//转储离线
	_offlineMsgModel.insert(channel, message);
}

// 用户注销
void ChatService::clientLogout(const TcpConnectionPtr &conn, json &js, Timestamp time) {
	int id = js["id"].get<int>();

	LOG_DEBUG("client id = %d start do logout !", id);

	clientCloseException(conn);
}