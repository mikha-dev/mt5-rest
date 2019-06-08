#include "stdafx.h"
#include "session_manager.hpp"

ManagersDatabase managerDB;
std::mutex DBMutex;

utility::string_t SessionManager:: save(const utility::string_t server, const utility::string_t login, const string_t password) {

	std::unique_lock<std::mutex> lock{ DBMutex };
	utility::string_t token;

	token.append(login);

	if (managerDB.find(token) != managerDB.end()) {
		return token;
	}

	ManagerInformation info;
	info.server = server;
	info.login = login;
	info.password = password;
	managerDB.insert(
		std::pair<utility::string_t, ManagerInformation>(token,
			info));

	return token;
}

bool SessionManager::check( const utility::string_t key, ManagerInformation & info) {

	if (managerDB.find(key) != managerDB.end()) {
		auto i = managerDB[key];
		info = i;
		return true;
	}

	return false;
}