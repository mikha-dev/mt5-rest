#include "stdafx.h"
#include <string>
#include <utility>
#include "safe_map.hpp"

string& SafeMap::operator[](string key) {
	return data[key];
}

void SafeMap::add(string key, string value) {
	lock_guard<mutex> lock(mut);
	data[key] = value;
	cond.notify_one();
}

bool SafeMap::contains(string key) {
	return data.count(key) > 0;
}

void SafeMap::remove(string key) {
	lock_guard<mutex> lock(mut);
	data.erase(key);
	cond.notify_one();
}