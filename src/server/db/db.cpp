#include "db.h"
#include <muduo/base/Logging.h>

// 数据库配置信息
static const string server = "127.0.0.1";
static const string user = "chat";
static const string password = "chat";
static const string dbname = "chat";

// 初始化数据库连接
MySql::MySql()
{
    _conn = mysql_init(nullptr);
}

// 释放数据库连接资源
MySql::~MySql()
{
    if (_conn != nullptr) {
        mysql_close(_conn);
    }
}

// 连接数据库
bool MySql::connect()
{
    MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(),
                                  password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    if (p != NULL)
    {
        // C和C++代码默认的编码字符是ASCII，如果不设置，从MySQL上拉下来的中文显示？
        mysql_query(_conn, "set names gbk");
    }
    else
    {
        LOG_INFO << "connect mysql failed!";
    }

    return p;   //返回nullptr代表连接失败
}

// 更新操作
bool MySql::update(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
                 << sql << "更新失败!";
        return false;
    }

    return true;
}

// 查询操作
MYSQL_RES *MySql::query(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
                 << sql << "查询失败!";
        return nullptr;
    }

    return mysql_use_result(_conn);
}

// 获取连接
MYSQL* MySql::getConnection()
{
    return _conn;
}