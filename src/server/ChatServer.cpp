#include "ChatServer.h"
#include "ChatService.h"
#include <string>
#include "utils/HvParse.h"
#include "utils/Log.h"
#include "json.hpp"

using json = nlohmann::json;
using namespace std;
using namespace hv;
using namespace placeholders;

ChatServer::ChatServer(const string &ip, int port, int threadNum) {
	int listen_fd = _server.createsocket(port, ip.c_str());
	if (listen_fd < 0) {
		LOG_ERROR("create socket error");
		return;
	}

	server_unpack_setting = new unpack_setting_t();
	memset(server_unpack_setting, 0, sizeof(unpack_setting_t));
	server_unpack_setting->mode = UNPACK_BY_LENGTH_FIELD;
	server_unpack_setting->package_max_length = DEFAULT_PACKAGE_MAX_LENGTH;
	server_unpack_setting->body_offset = SERVER_HEAD_LENGTH;
	server_unpack_setting->length_field_offset = SERVER_HEAD_LENGTH_FIELD_OFFSET;
	server_unpack_setting->length_field_bytes = SERVER_HEAD_LENGTH_FIELD_BYTES;
	server_unpack_setting->length_field_coding = ENCODE_BY_BIG_ENDIAN;
	_server.setUnpack(server_unpack_setting);
	_server.setThreadNum(threadNum);

	_server.onConnection = [this](const hv::SocketChannelPtr &conn) {
	  onConnection(conn);
	};
	_server.onMessage = [this](const hv::SocketChannelPtr &conn, hv::Buffer *buf) {
	  onMessage(conn, buf);
	};

	// 连接 MySQL

	// 连接 Redis
}

void ChatServer::onConnection(const hv::SocketChannelPtr &conn) {
	std::string peerAddr = conn->peeraddr();
	if (conn->isConnected()) {
		LOG_INFO("new connection: %s , connfd = %d", peerAddr.c_str(), conn->fd());
	} else {
		LOG_INFO("connection closed: %s , connfd = %d", peerAddr.c_str(), conn->fd());
	}
}

void ChatServer::onMessage(const SocketChannelPtr &conn, hv::Buffer *buf) {
	std::string data = std::string((char *)buf->data(), buf->size());
	auto message = HvParse::unpackMessage(data);
	json reply = json::parse(message);
	auto msgHandler = ChatService::instance()->getHandler(reply["msgid"].get<int>());

	msgHandler(conn, reply);
}

void ChatServer::start() {
	_server.start();
	LOG_INFO("server start");
	while (getchar() != '\n');

}
