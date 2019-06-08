#ifndef SAFEMAP_HPP
#define SAFEMAP_HPP

#include <map>
#include <mutex>
#include <condition_variable>
#include <string>

using namespace std;

class SafeMap {
public:
	SafeMap() : data(), mut(), cond() {}
	SafeMap(const SafeMap& orig) : data(orig.data), mut(), cond() {}
	~SafeMap() {}

	void add(string key, string value);
	string& operator[](const string key);
	void remove(string key);
	bool contains(string key);

private:
	map<string,string> data;
	mutex mut;
	condition_variable cond;
};

#endif /* SAFEMAP_HPP */