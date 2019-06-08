#pragma once

#include <map>
#include <cpprest/details/basic_types.h>
#include <boost/uuid/detail/md5.hpp>

using namespace utility;

typedef struct {
	utility::string_t server;
	utility::string_t login;
	utility::string_t password;
} ManagerInformation;

using ManagersDatabase = std::map<utility::string_t, ManagerInformation>;

class SessionManager {
public:
	utility::string_t save(const utility::string_t server, const utility::string_t login, const utility::string_t password);
	bool check(const utility::string_t key, ManagerInformation & managerInfo);
};