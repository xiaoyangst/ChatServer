#include "FriendModel.h"
#include "dbpool/MySQLConnectionPool.h"

void FriendModel::insert(int userId, int friendId) {
	char sql[1024] = {0};
	snprintf(sql, sizeof(sql), "insert into friend values(%d,%d)", userId, friendId);

	MySQLConnectionPool::getInstance()->getConnection()->update(sql);

//    MySql mysql;
//    if (mysql.connect()) {
//        mysql.update(sql);
//    }
}

std::vector<User> FriendModel::query(int userId) {
	char sql[1024] = {0};

	//联合查询，因为该表中并没有用户的实际数据，但能通过用户ID
	sprintf(sql, "select a.id, a.name, a.state from user a inner join friend b on b.friendid = a.id where b.userid=%d",
			userId);

	std::vector<User> vec;

	MYSQL_RES *res = MySQLConnectionPool::getInstance()->getConnection()->query_result(sql);
	if (res != nullptr) {
		// 把userid用户的所有离线消息放入vec中返回
		MYSQL_ROW row;
		while ((row = mysql_fetch_row(res)) != nullptr) {
			User user;
			user.setId(atoi(row[0]));
			user.setName(row[1]);
			user.setState(row[2]);
			vec.push_back(user);
		}
		mysql_free_result(res);
		return vec;
	}

	return vec;

}

