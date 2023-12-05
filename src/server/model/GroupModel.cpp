#include "GroupModel.h"
#include "db.h"

//创建群，名称+描述
bool GroupModel::createGroup(Group &group) {
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "insert into allgroup(groupname, groupdesc) values('%s', '%s')",
             group.getName().c_str(), group.getDesc().c_str());

    MySql mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }

    return false;
}

//加入群，用户入群（用户id+群id+角色）
void GroupModel::addGroup(int userid, int groupid, std::string role) {
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "insert into groupuser values(%d, %d, '%s')",
             groupid, userid, role.c_str());

    MySql mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

//查询用户在群组信息
std::vector<Group> GroupModel::queryGroups(int userid) {
    //先查询用户所在的群组有哪些
    //再从这些群组信息中获取这个用户的信息

    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "select a.id,a.groupname,a.groupdesc from allgroup a inner join \
        groupuser b on a.id = b.groupid where b.userid=%d",
             userid);

    std::vector<Group> groupVec;

    MySql mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            // 查出userid所有的群组信息，保存在vector容器中
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                groupVec.push_back(group);
            }
            mysql_free_result(res);
        }
    }

    //根据收集到的群组信息，开始返回用户群组的相关信息
    for (Group &group : groupVec)
    {
        snprintf(sql, sizeof(sql), "select a.id,a.name,a.state,b.grouprole from user a \
            inner join groupuser b on b.userid = a.id where b.groupid=%d",
                 group.getId());

        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                GroupUser user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setRole(row[3]);
                group.getUsers().push_back(user);
            }
            mysql_free_result(res);
        }
    }
    return groupVec;
}

// 根据指定的groupid查询群组用户id列表，除userid自己，主要用户群聊业务给群组其它成员群发消息
std::vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
{
    char sql[1024] = {0};
    sprintf(sql, "select userid from groupuser where groupid = %d and userid != %d", groupid, userid);

    vector<int> idVec;
    MySql mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                idVec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return idVec;
}