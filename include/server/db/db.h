//
// 数据库编程常用API封装
//

#ifndef CHATSERVER_DB_H
#define CHATSERVER_DB_H

#include <mysql/mysql.h>
#include <string>
using namespace std;

// 数据库操作类
class MySql
{
public:
    // 初始化数据库连接
    MySql();
    // 释放数据库连接资源
    ~MySql();
    // 连接数据库
    bool connect();
    // 更新操作
    bool update(string sql);
    // 查询操作
    MYSQL_RES *query(string sql);
    // 获取连接
    MYSQL* getConnection();

private:
    MYSQL *_conn;
};


#endif //CHATSERVER_DB_H
