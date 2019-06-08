#pragma once
#include "stdafx.h"

using namespace std;
using jtype = json::value::value_type;

class FormatException : public std::exception {
	char message[100];
public:
	FormatException(const utility::string_t field, const utility::string_t type) {
		utility::string_t t(field);
		t.append(L" should be ");
		t.append(type);

		size_t i;
		wcstombs_s(&i, message, t.length() + 1, t.c_str(), t.length());
	}
	const char * what() const throw() {
		return message;
	}
};

class ManagerException : public std::exception {
	char description[100];

public:
	int code;
	ManagerException(const int code_, const char * description_) {
		code = code_;
		strncpy_s(description, description_, 100);
	}

	const char * what() const throw() {
		return description;
	}

};

class RequiredException : public std::exception {
	char message[100];
public:
	RequiredException(const utility::string_t field) {
		utility::string_t t(field);
		t.append(L" is required");

		size_t i;
		wcstombs_s(&i, message, t.length() + 1, t.c_str(), t.length());
	}
	const char * what() const throw() {
		return message;
	}
};

struct Error {
	utility::string_t description;
	int code;

	web::json::value AsJSON() const
	{
		web::json::value result = web::json::value::object();

		result[U("descr")] = web::json::value::string(description);
		result[U("code")] = web::json::value::number(code);

		return result;
	}
};