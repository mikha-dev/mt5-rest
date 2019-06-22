#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include "stdafx.h"
#include "microsvc_controller.hpp"
#include "types.hpp"

#define CMD_DOCS L"docs"
#define CMD_SUB L"sub"
#define CMD_SWAGGER L"swagger.json"
#define CMD_VERSION L"version"

using namespace web;
using namespace http;

void MicroserviceController::initRestOpHandlers() {
	_listener.support(methods::GET, std::bind(&MicroserviceController::handleGet, this, std::placeholders::_1));
	_listener.support(methods::PUT, std::bind(&MicroserviceController::handlePut, this, std::placeholders::_1));
	_listener.support(methods::POST, std::bind(&MicroserviceController::handlePost, this, std::placeholders::_1));
	_listener.support(methods::DEL, std::bind(&MicroserviceController::handleDelete, this, std::placeholders::_1));
	_listener.support(methods::PATCH, std::bind(&MicroserviceController::handlePatch, this, std::placeholders::_1));
	_listener.support(methods::HEAD, std::bind(&MicroserviceController::handleHead, this, std::placeholders::_1));

	pushCommand(L"inited", endpoint());
}

void MicroserviceController::pushCommand(string_t command, string_t options) {

	web::json::value result = web::json::value::object();

	result[U("command")] = web::json::value::string(command);
	result[U("options")] = web::json::value::string(options);

	commands.push_back(ws2s(result.serialize()));
}

const char* MicroserviceController::getCommand() {
	string command;

	if (commands.size() < 1)
		return NULL;

	command.append(commands.back());
	commands.pop_back();

	return command.c_str();
}

int MicroserviceController::hasCommands() {
	return commands.size() > 0;
}


void MicroserviceController::setCommandResponse(const char* command, const char* response) {
	commandResponses[command] = response;
}

void MicroserviceController::setCallback(const char* url, const char* format) {
	callback_url.clear();
	callback_format.clear();
	callback_url.append(s2ws(url));
	callback_format.append(s2ws(format));	
}

void MicroserviceController::setCommandWaitTimeout(int timeout) {
	wait_timeout = timeout*1000;
}

void MicroserviceController::setPath(const char *_path, const char* _url_swagger) {
	path_docs.clear();
	path_docs.append(_path);

	url_swagger.clear();
	url_swagger.append(_url_swagger);
}

int MicroserviceController::onEvent(const char* data) {

	if (callback_url.length() < 1)
		return -1;

	Concurrency::task<web::http::http_response> task;
	http_client callback_client(callback_url);

	try {

		if (callback_format == L"json") {

			task = callback_client.request(methods::POST, "", data);
		}
		else {
			http_request request(methods::POST);
			request.headers().add(L"Content-Type", L"application/x-www-form-urlencoded; charset=UTF-8");
			request.set_body(data);
			task = callback_client.request(request);
		}

		task.then([](http_response response) {
			//ucout << response.to_string();
			if (response.status_code() == status_codes::OK) {
				auto body = response.extract_string().get();
				ucout << body << std::endl;
			}
		}).wait();

		return 1;
	}
	catch (const web::http::http_exception &e) {
		ucout << e.error_code() << endl;
	}
	catch (const std::exception & ex) {
		ucout << ex.what() << endl;
	}
	catch (...) {
	}

	return -1;
}

auto MicroserviceController::formatError(int code, const utility::string_t message) {
	web::json::value result = web::json::value::object();

	result[U("message")] = web::json::value::string(message);
	result[U("code")] = web::json::value::number(code);

	return result;
}

auto MicroserviceController::formatError(int code, const char* message) {
	wstring msg(message, message + strlen(message));

	return formatError(code, msg);
}

auto MicroserviceController::formatErrorRequired(utility::string_t field) {
	utility::string_t msg(field);

	msg.append(U(" is required"));

	return formatError(402, msg);
}

void MicroserviceController::handleGet(http_request message) {
	auto response = json::value::object();
	auto path = requestPath(message);
	auto params = requestQueryParams(message);

	try {

		if (path.size() < 1) {
			string p(path_docs);
			p.append("docs.html");
			std::ifstream in(p, ios::in);

			http_response response(status_codes::OK);
			response.headers().add(L"Content-Type", L"text/html; charset=UTF-8");

			std::stringstream buffer;
			buffer << in.rdbuf();
			string b = buffer.str();
			in.close();

			response.set_body(b);
			message.reply(response);

			return;
		}

		if (path[0] == CMD_SWAGGER) {
			string p(path_docs);
			p.append("swagger.json");
			std::ifstream in(p, ios::in);

			http_response response(status_codes::OK);
			response.headers().add(L"Content-Type", L"text/json; charset=UTF-8");

			std::stringstream buffer;
			buffer << in.rdbuf();
			string b = buffer.str();
			in.close();

			boost::algorithm::replace_all<string,string,string>(b, "localhost:6542", url_swagger);

			response.set_body(b);

			message.reply(response);
			
			return;
		}

		if (!token.empty()) {
			if (!message.headers().has(header_names::authorization)) {
				message.reply(status_codes::Unauthorized);
				return;
			}

			auto headers = message.headers();

			if (headers[header_names::authorization] != token) {
				message.reply(status_codes::Unauthorized);
				return;
			}
		}

		web::json::value result = web::json::value::object();

		result[L"command"] = web::json::value::string(path[0]);

		if (path.size() > 1) {
			result[L"id"] = web::json::value::string(path[1]);
		}

		for (auto it = params.begin(); it != params.end(); ++it) {
			result[it->first] = web::json::value::string(it->second);
		}

		string command = ws2s(result.serialize());
		commands.push_back(command);

		DWORD dw1 = GetTickCount();

		while(dw1 + wait_timeout > GetTickCount()) {

			if (commandResponses.contains(command)) {					
				message.reply(status_codes::OK, commandResponses[command], "application/json");
				commandResponses.remove(command);
				return;
			}

			Sleep(1);
		}

		throw exception("Failed to get info, timeout");
	}
	catch (const FormatException & e) {
		message.reply(status_codes::BadRequest, formatError(405, e.what()));
	}
	catch (const RequiredException & e) {
		message.reply(status_codes::BadRequest, formatError(405, e.what()));
	}
	catch (const json::json_exception & e) {
		message.reply(status_codes::BadRequest, formatError(410, e.what()));
		ucout << e.what() << endl;
	}
	catch (const std::exception & ex) {
		message.reply(status_codes::BadRequest, formatError(410, ex.what()));
		ucout << ex.what() << endl;
	}
	catch (...) {
		message.reply(status_codes::InternalError);
	}
}

void MicroserviceController::handlePost(http_request message) {
	auto response = json::value::object();

	try {

		auto path = requestPath(message);
		auto params = requestQueryParams(message);

		ucout << message.to_string() << endl;

		if (path.size() < 1) {
			message.reply(status_codes::NotFound);
			return;
		}

		if (!token.empty()) {
			if (!message.headers().has(header_names::authorization)) {
				message.reply(status_codes::Unauthorized);
				return;
			}

			auto headers = message.headers();

			if (headers[header_names::authorization] != token) {
				message.reply(status_codes::Unauthorized);
				return;
			}
		}

		if (path[0] == CMD_SUB) {
			callback_url = params[U("callback_url")];
			callback_format = params[U("callback_format")];

			response[U("message")] = json::value::string(L"Succesfully subscribed");
			response[U("code")] = json::value::number(200);
			message.reply(status_codes::OK, response);

			return;
		}

		message.extract_utf8string(true).then([=](std::string body) {

			if (body.length() < 1) {
				throw exception("POST body is empty");
			}

			std::size_t pos = body.find("}");
			std::string command = body.substr(0, pos);

			command.append(",\"command\":\"");
			command.append(ws2s(path[0]));
			command.append("\"}");

			commands.push_back(command);

			for (int i = 0; i < wait_timeout; i++) {
				if (commandResponses.contains(command)) {
					message.reply(status_codes::OK, commandResponses[command], "application/json");
					commandResponses.remove(command);
					return;
				}

				Sleep(1);
			}

			throw exception("Failed to get info, timeout");
		}).wait();

	}
	catch (const ManagerException & e) {
		message.reply(status_codes::BadRequest, formatError(e.code, e.what()));
	}
	catch (const FormatException & e) {
		message.reply(status_codes::BadRequest, formatError(405, e.what()));
	}
	catch (const RequiredException & e) {
		message.reply(status_codes::BadRequest, formatError(405, e.what()));
	}
	catch (const json::json_exception & e) {
		message.reply(status_codes::BadRequest, formatError(410, e.what()));
		ucout << e.what() << endl;
	}
	catch (const std::exception & ex) {
		message.reply(status_codes::BadRequest, formatError(410, ex.what()));
		ucout << ex.what() << endl;
	}
	catch (...) {
		message.reply(status_codes::InternalError);
	}

}

void MicroserviceController::handleDelete(http_request message) {
	try {
		auto path = requestPath(message);
		auto params = requestQueryParams(message);

		if (path.size() < 1) {
			message.reply(status_codes::NotFound);
			return;
		}

	} 
	catch (const ManagerException & e) {
		message.reply(status_codes::BadRequest, formatError(e.code, e.what()));
	}
	catch (const FormatException & e) {
		message.reply(status_codes::BadRequest, formatError(405, e.what()));
	}
	catch (const RequiredException & e) {
		message.reply(status_codes::BadRequest, formatError(405, e.what()));
	}
	catch (const json::json_exception & e) {
		message.reply(status_codes::BadRequest, formatError(410, e.what()));
		ucout << e.what() << endl;
	}
	catch (const std::exception & ex) {
		message.reply(status_codes::BadRequest, formatError(410, ex.what()));
		ucout << ex.what() << endl;
	}
	catch (...) {
		message.reply(status_codes::InternalError);
	}
}

void MicroserviceController::handleHead(http_request message) {
	auto response = json::value::object();
	response[U("version")] = json::value::string(U("0.1.1"));
	response[U("code")] = json::value::number(200);
	message.reply(status_codes::OK, "version");
}

void MicroserviceController::handleOptions(http_request message) {
	http_response response(status_codes::OK);
	response.headers().add(U("Allow"), U("GET, POST, OPTIONS"));
	response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
	response.headers().add(U("Access-Control-Allow-Methods"), U("GET, POST, OPTIONS"));
	response.headers().add(U("Access-Control-Allow-Headers"), U("Content-Type"));
	message.reply(response);
}

void MicroserviceController::handleTrace(http_request message) {
	message.reply(status_codes::NotImplemented, responseNotImpl(methods::TRCE));
}

void MicroserviceController::handleConnect(http_request message) {
	message.reply(status_codes::NotImplemented, responseNotImpl(methods::CONNECT));
}

void MicroserviceController::handleMerge(http_request message) {
	message.reply(status_codes::NotImplemented, responseNotImpl(methods::MERGE));
}

void MicroserviceController::handlePatch(http_request message) {
	message.reply(status_codes::NotImplemented, responseNotImpl(methods::MERGE));
}

void MicroserviceController::handlePut(http_request message) {
	message.reply(status_codes::NotImplemented, responseNotImpl(methods::MERGE));
}

json::value MicroserviceController::responseNotImpl(const http::method & method) {

	using namespace json;

	auto response = value::object();
	response[U("serviceName")] = value::string(U("MT4 REST"));
	response[U("http_method")] = value::string(method);

	return response;
}
