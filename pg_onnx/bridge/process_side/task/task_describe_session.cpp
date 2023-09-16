//
// Created by Kibae Shin on 2023/09/13.
//

#include "task.hpp"

#include "pg_srf_macro.h"
#include "session_response_macro.h"

Datum task_describe_session(
	extension_state_t *state, PG_FUNCTION_ARGS, const std::string &name, const std::string &version
) {
	PG_SRF_BEGIN() {
		// process
		auto request = json::object();
		request["model"] = name;
		request["version"] = version;

		auto result = api_request(state, Orts::task::type::GET_SESSION, request);
		if (result.is_object() && result.contains("error"))
			throw std::runtime_error(result["error"].get<std::string>());

		funcctx->user_fctx = (void *)new json_list_iterator(json::array({result}));
	}
	PG_SRF_END();

	RESPONSE_SESSION_LIST
}
