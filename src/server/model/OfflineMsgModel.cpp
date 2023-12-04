#include "OfflineMsgModel.h"
#include "db.h"

void OfflineMsgModel::insert(int userId, std::string msg) {
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "insert into offlinemessage values(%d, '%s')", userId, msg.c_str());

    MySql mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

void OfflineMsgModel::remove(int userId) {
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "delete from offlinemessage where userid=%d", userId);

    MySql mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

std::vector<std::string> OfflineMsgModel::query(int userId) {
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "select message from offlinemessage where userid = %d", userId);

    std::vector<std::string> vec;
    MySql mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            // 把userid用户的所有消息放入vec中返回
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                vec.push_back(row[0]);
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}


