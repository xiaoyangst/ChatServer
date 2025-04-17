#include <iostream>
#include <vector>
#include <cmath>
#include <sstream>
#include <map>
#include <cstdint>
#include <unordered_map>
#include <set>
#include <algorithm>

using namespace std;

void DFS(const vector<int> &data, int index) {
	
}

int main() {

	std::string line;
	getline(cin, line);
	istringstream iss(line);
	vector<int> data;
	data.push_back(0);
	std::string num;
	while (getline(iss, num, ' ')) {
		data.push_back(stoi(num));
	}

	return 0;
}

