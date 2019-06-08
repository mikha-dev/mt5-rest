#include "stdafx.h"
#include "network_utils.hpp"    // -> these two lines must stay in this order, boost/cpprestsdk header include order breaks "U"
#include "basic_controller.hpp" // -|

namespace cfx {
	BasicController::BasicController() {

	}

	BasicController::~BasicController() {

	}
	void BasicController::setEndpoint(const utility::string_t & value) {
		uri endpointURI(value);
		uri_builder endpointBuilder;

		endpointBuilder.set_scheme(endpointURI.scheme());

		if (endpointURI.host() == U("host_auto_ip4")) {
			endpointBuilder.set_host(NetworkUtils::hostIP4());
		}
		else if (endpointURI.host() == U("host_auto_ip6")) {
			endpointBuilder.set_host(NetworkUtils::hostIP6());
		}
		else {
			endpointBuilder.set_host(endpointURI.host());
		}
		endpointBuilder.set_port(endpointURI.port());
		endpointBuilder.set_path(endpointURI.path());

		_listener = http_listener(endpointBuilder.to_uri());
	}

	utility::string_t BasicController::endpoint() const {
		return _listener.uri().to_string();
	}

	utility::string_t BasicController::getHostPort() const {
		return _listener.uri().host() + L":" + std::to_wstring( _listener.uri().port() );
	}

	pplx::task<void> BasicController::accept() {
		initRestOpHandlers();
		return _listener.open();
	}

	pplx::task<void> BasicController::shutdown() {
		return _listener.close();
	}

	std::vector<utility::string_t> BasicController::requestPath(const http_request & message) {
		auto relativePath = uri::decode(message.relative_uri().path());
		return uri::split_path(relativePath);
	}

	std::map<utility::string_t, utility::string_t> BasicController::requestQueryParams(const http_request & message) {
		auto query = message.relative_uri().query();
		return uri::split_query(query);
	}
}
