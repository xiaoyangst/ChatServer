#if 0

#include "GroupModel.h"
#include <iostream>

using namespace std;

int main(){
    GroupModel groupModel;
    std::vector<Group> res = groupModel.queryGroups(18);
    for (auto &i : res) {
        cout<<"id = "<<i.getId()<<" name = "<<i.getName()<<" desc = "<<i.getDesc()<<endl;
    }
    return 0;
}

#endif