//
// Created by Kibae Shin on 2023/09/13.
//

#include "task.hpp"

Datum task_execute_session(
	extension_state_t *state, PG_FUNCTION_ARGS, const std::string &name, const std::string &version, const json &inputs
) {
	auto request = json::object();
	request["model"] = name;
	request["version"] = version;
	request["data"] = inputs;

	auto result = api_request(state, Orts::task::type::EXECUTE_SESSION, request);
	if (result.is_object() && result.contains("error")) {
		if (result["error"].is_string() && result["error"] == "session not found") {
			// session not found. create session and retry.
			create_session(state, name, version);
			result = api_request(state, Orts::task::type::EXECUTE_SESSION, request);
			if (result.is_object() && result.contains("error"))
				throw std::runtime_error(result["error"].get<std::string>());
		} else
			throw std::runtime_error(result["error"].get<std::string>());
	}

	// std::cout << "inputs: " << inputs.dump(2) << std::endl;
	// std::cout << "outputs: " << result.dump(2) << std::endl;

	Datum jsonb_datum = DirectFunctionCall1(jsonb_in, CStringGetDatum(result.dump().c_str()));

	PG_RETURN_JSONB_P(DatumGetJsonbP(jsonb_datum));
}
