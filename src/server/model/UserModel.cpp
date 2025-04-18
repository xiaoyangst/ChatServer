#include "UserModel.h"
#include <iostream>
#include "dbpool/MySQLConnectionPool.h"

User UserModel::qurry(int id) {
	//构建sql查询语句
	char sql[1024] = {0};
	snprintf(sql, sizeof(sql), "select * from user where id = %d", id);

	//建立连接，返回查询结果（User）

	MYSQL_RES *res = MySQLConnectionPool::getInstance()->getConnection()->query_result(sql);
	if (res != nullptr) {
		MYSQL_ROW row = mysql_fetch_row(res);
		if (row != nullptr) {
			User user;
			user.setId(atoi(row[0]));
			user.setName(row[1]);
			user.setPassword(row[2]);
			user.setState(row[3]);
			mysql_free_result(res);
			return user;
		}
	}

	return User();
}

bool UserModel::insert(User &user) {
	//构建sql查询语句
	char sql[1024] = {0};
	//不用id参数的原因是，id在MySQL数据库已被我们设置为自动增长
	snprintf(sql, sizeof(sql), "insert into user(name, password, state) values('%s', '%s', '%s')",
			 user.getName().c_str(), user.getPassword().c_str(), user.getState().c_str());

	auto conn = MySQLConnectionPool::getInstance()->getConnection();
	if (conn->update(sql)) { //插入成功
		//更新id，但不能乱传参
		//mysql_insert_id 用于获取最后插入行的 ID。该函数返回最后插入行的 ID，该 ID 由一个 AUTO_INCREMENT 列生成
		user.setId(mysql_insert_id(conn->getConnSrc()));
		return true;
	}

	return false;
}

//更新操作，用户状态信息
bool UserModel::updateState(User user) {
	char sql[1024] = {0};
	snprintf(sql, sizeof(sql), "update user set state = '%s' where id =%d", user.getState().c_str(), user.getId());

	if (MySQLConnectionPool::getInstance()->getConnection()->update(sql)) {
		return true;
	}

	return false;
}

void UserModel::resetState() {
	char sql[1024] = {0};
	snprintf(sql, sizeof(sql), "update user set state = 'offline' where state = 'online'");

	MySQLConnectionPool::getInstance()->getConnection()->update(sql);
}