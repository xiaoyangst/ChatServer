#include "ChatService.h"

#include "public.h"
#include "Group.h"
#include <iostream>
#include "utils/Log.h"

using namespace placeholders;

//注册消息以及对应的Handle对应的回调函数
ChatService::ChatService() {
	_msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::loginHandler, this, _1, _2)});
	_msgHandlerMap.insert({REGISTER_MSG, std::bind(&ChatService::registerHandler, this, _1, _2)});
	_msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChatHandler, this, _1, _2)});
	_msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriendHandler, this, _1, _2)});
	_msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2)});
	_msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2)});
	_msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::chatGroup, this, _1, _2)});
	_msgHandlerMap.insert({LOGINOUT_MSG, std::bind(&ChatService::clientLogout, this, _1, _2)});

	auto re = RedisConnectionPool::getInstance();
	re->init(std::bind(&ChatService::redis_subscribe_message_handler, this, _1, _2));

	cout << "初始化完成" << endl;
}

MsgHandler ChatService::getHandler(int msgId) {
	auto it = _msgHandlerMap.find(msgId);
	if (it != _msgHandlerMap.end()) {
		return _msgHandlerMap[msgId];
	} else {
		return [=](const hv::SocketChannelPtr &conn, json &js) {
		  LOG_ERROR ("msgId: %d can not find handler!", msgId);
		};
	}
}

//测试通过
void ChatService::registerHandler(const hv::SocketChannelPtr &conn, json &js) {
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

		//dump 方法把 json 转换为 std::string
		conn->write(response.dump());
		cout << "register info to client" << endl;
	} else {
		json response;
		response["msgid"] = REGISTER_MSG_ACK;
		response["errno"] = 1;
		// 注册已经失败，不需要在 json 返回 id
		conn->write(response.dump());
	}

}

//测试通过
void ChatService::loginHandler(const hv::SocketChannelPtr &conn, json &js) {
	LOG_INFO("start do login !");

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
			conn->write(response.dump());
		} else { //登录成功

			LOG_INFO("%s:login success!", user.getName().c_str());

			{
				lock_guard<mutex> lockGuard(_conmutex);
				_userConnMap.insert({id, conn});
			}

			// id 用户登录成功后，向 redis 订阅 channel(id)
			RedisConnectionPool::getInstance()->getConn()->subscribe(std::to_string(id));

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

			conn->write(response.dump());
		}
	} else {  //登录失败
		json response;
		response["msgid"] = LOGIN_MSG_ACK;
		response["errno"] = -1;
		response["errmsg"] = "login failed ,Try again!\n";
		conn->write(response.dump());
	}
}

//测试通过
void ChatService::clientCloseException(const hv::SocketChannelPtr &conn) {
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
	RedisConnectionPool::getInstance()->getConn()->unsubscribe(to_string(user.getId()));

	//更新用户状态
	if (user.getId() != -1) {
		user.setState("offline");
		_userModel.updateState(user);
	}
}

//如果用户登录状态，直接给他发消息
//如果用户离线状态，把离线消息存储起来，等他上线时候读取
void ChatService::oneChatHandler(const hv::SocketChannelPtr &conn, json &js) {
	//先获取 聊天对象的ID
	int toId = js["toid"].get<int>();
	cout << "Server Buffer " << js.dump() << endl;
	{
		lock_guard<mutex> lockGuard(_conmutex);
		auto it = _userConnMap.find(toId);
		if (it != _userConnMap.end()) {  //确认在线
			it->second->write(js.dump());
			return;
		}
	}

	// 用户在其他主机的情况，publish消息到 redis
	User user = _userModel.qurry(toId);
	if (user.getState() == "online") {
		RedisConnectionPool::getInstance()->getConn()->publish(to_string(toId), js.dump());
		return;
	}

	//处理离线
	_offlineMsgModel.insert(toId, js.dump());
}

//测试通过
void ChatService::reset() {
	//全部online转换为offline
	_userModel.resetState();
}

//测试通过
void ChatService::addFriendHandler(const hv::SocketChannelPtr &conn, json &js) {
	int userId = js["id"].get<int>();
	int friendId = js["friendid"].get<int>();

	_friendModel.insert(userId, friendId);
}

//测试通过
void ChatService::createGroup(const hv::SocketChannelPtr &conn, json &js) {
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

//测试通过
void ChatService::addGroup(const hv::SocketChannelPtr &conn, json &js) {
	int userId = js["id"].get<int>();
	int groupId = js["groupid"].get<int>();
	_groupModel.addGroup(userId, groupId, "normal");
}

void ChatService::chatGroup(const hv::SocketChannelPtr &conn, json &js) {
	int userId = js["id"].get<int>();
	int groupId = js["groupid"].get<int>();
	std::vector<int> userIdVec = _groupModel.queryGroupUsers(userId, groupId);

	lock_guard<mutex> lock(_conmutex);
	for (int id : userIdVec) {
		auto it = _userConnMap.find(id);
		if (it != _userConnMap.end()) {
			it->second->write(js.dump());
		} else {
			User user = _userModel.qurry(id);
			if (user.getState() == "online") {    // 发布消息，给其他服务器上的用户
				RedisConnectionPool::getInstance()->getConn()->publish(to_string(id), js.dump());
				std::cout << "publish" << std::endl;
			} else {
				_offlineMsgModel.insert(id, js.dump());
			}
		}
	}
}

// redis订阅消息触发的回调函数,这里channel其实就是id
void ChatService::redis_subscribe_message_handler(const string &channel, const string &message) {
	//用户在线
	lock_guard<mutex> lock(_conmutex);
	auto it = _userConnMap.find(stoi(channel));
	if (it != _userConnMap.end()) {
		it->second->write(message);
		return;
	}

	//转储离线
	_offlineMsgModel.insert(stoi(channel), message);
}

//测试通过
void ChatService::clientLogout(const hv::SocketChannelPtr &conn, json &js) {
	int id = js["id"].get<int>();

	LOG_DEBUG("client id = %d start do logout !", id);

	clientCloseException(conn);
}