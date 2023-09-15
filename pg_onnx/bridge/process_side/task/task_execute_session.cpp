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
	if (result.is_object() && result.contains("error"))
		throw std::runtime_error(result["error"].get<std::string>());

	// std::cout << "inputs: " << inputs.dump() << std::endl;
	// std::cout << "outputs: " << result.dump(2) << std::endl;

	text *resultText = cstring_to_text(result.dump().c_str());
	Datum jsonbDatum = DirectFunctionCall1(jsonb_in, CStringGetDatum(text_to_cstring(resultText)));

	PG_RETURN_JSONB_P(DatumGetJsonbP(jsonbDatum));
}
