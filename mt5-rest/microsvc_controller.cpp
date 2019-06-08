#include "stdafx.h"
#include "microsvc_controller.hpp"
#include "types.hpp"

#define CMD_VERSION L"version"
#define CMD_SUB L"sub"

using namespace web;
using namespace http;

void MicroserviceController::initRestOpHandlers() {
	_listener.support(methods::GET, std::bind(&MicroserviceController::handleGet, this, std::placeholders::_1));
	_listener.support(methods::PUT, std::bind(&MicroserviceController::handlePut, this, std::placeholders::_1));
	_listener.support(methods::POST, std::bind(&MicroserviceController::handlePost, this, std::placeholders::_1));
	_listener.support(methods::DEL, std::bind(&MicroserviceController::handleDelete, this, std::placeholders::_1));
	_listener.support(methods::PATCH, std::bind(&MicroserviceController::handlePatch, this, std::placeholders::_1));
	_listener.support(methods::HEAD, std::bind(&MicroserviceController::handleHead, this, std::placeholders::_1));
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

void MicroserviceController::setCommandWaitTimeout(int timeout) {
	wait_timeout = timeout*1000;
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
			message.reply(status_codes::NotFound);
			return;
		}
		if (path[0] == CMD_VERSION) {
			response[U("version")] = json::value::string(U("0.1.1"));
			response[U("code")] = json::value::number(200);
			message.reply(status_codes::OK, response);

			return;
		}

		string command = "{\"command\":\"";

		command.append(ws2s( path[0] ));
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
	message.reply(status_codes::NotImplemented, responseNotImpl(methods::OPTIONS));
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
