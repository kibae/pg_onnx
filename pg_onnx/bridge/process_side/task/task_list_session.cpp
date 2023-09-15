//
// Created by Kibae Shin on 2023/09/13.
//

#include "task.hpp"

#include "pg_srf_macro.h"
#include "session_response_macro.h"

Datum task_list_session(extension_state_t *state, PG_FUNCTION_ARGS) {
	PG_SRF_BEGIN() {
		// process
		auto result = api_request(state, Orts::task::type::LIST_SESSION, "");
		if (result.is_object() && result.contains("error"))
			throw std::runtime_error(result["error"].get<std::string>());
		if (!result.is_array())
			throw std::runtime_error("invalid response");

		funcctx->user_fctx = (void *)new json_list_iterator(result);
	}
	PG_SRF_END();

	RESPONSE_SESSION_LIST
}
