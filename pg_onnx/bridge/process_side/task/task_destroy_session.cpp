//
// Created by Kibae Shin on 2023/09/13.
//

#include "task.hpp"

Datum task_destroy_session(
	extension_state_t *state, PG_FUNCTION_ARGS, const std::string &name, const std::string &version
) {
	auto request = json::object();
	request["model"] = name;
	request["version"] = version;

	auto result = api_request(state, Orts::task::type::DESTROY_SESSION, request);
	if (result.is_object() && result.contains("error"))
		throw std::runtime_error(result["error"].get<std::string>());

	PG_RETURN_BOOL(result.get<bool>());
}
