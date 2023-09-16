//
// Created by Kibae Shin on 2023/09/13.
//

#ifndef PG_ONNX_TASK_HPP
#define PG_ONNX_TASK_HPP

#include "../process_side.hpp"

Datum task_list_session(extension_state_t *state, PG_FUNCTION_ARGS);
json create_session(extension_state_t *state, const std::string &name, const std::string &version);
Datum task_create_session(
	extension_state_t *state, PG_FUNCTION_ARGS, const std::string &name, const std::string &version
);
Datum task_execute_session(
	extension_state_t *state, PG_FUNCTION_ARGS, const std::string &name, const std::string &version, const json &inputs
);
Datum task_destroy_session(extension_state_t *state, PG_FUNCTION_ARGS, const std::string &name, const std::string &version);

#endif // PG_ONNX_TASK_HPP
