#if 0

#include "UserModel.h"
#include <iostream>
using namespace std;

int main(){

    UserModel userModel;

    //qurry 测试
    cout<<"《《 qurry测试 》》"<<endl;
    User user = userModel.qurry(13);

    cout<<"id = "<<user.getId()<<" name = "<<user.getName()<<" password = "<<user.getPassword()<<" state = "<<user.getState()<<endl;

    //insert测试
    cout<<"《《 insert测试 》》"<<endl;
    User InsertUser;

    InsertUser.setName("xy");
    InsertUser.setPassword("123");
    InsertUser.setState("offline");

    userModel.insert(InsertUser);


    //updateState
    cout<<"《《 updateState测试 》》"<<endl;
    cout<<"重置前 state = "<<user.getState()<<endl;
    user.setState("online");
    userModel.updateState(user);
    cout<<"重置后 state = "<<user.getState()<<endl;

    //resetState测试
    cout<<"《《 resetState测试 》》"<<endl;
    cout<<"重置前 state = "<<user.getState()<<endl;
    userModel.resetState();

    User user1 =  userModel.qurry(2);
    cout<<"重置后 state = "<<user1.getState()<<endl;


    return 0;
}

#endif