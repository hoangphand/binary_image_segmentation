#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <queue>
#include <stack>
#include <list>
using namespace std;

int main(int argc, char const *argv[]) {
	// stack<int> a;
	// queue<int> a;
	list<int> a;
	// a.push_back(1);
	// a.push_back(2);
	// a.push_back(3);
	// a.push_back(4);
	a.push_front(1);
	a.push_front(2);
	a.push_front(3);
	a.push_front(4);
	// cout<<a.top();
	// a.pop();
	cout<<a.front();
	return 0;
}