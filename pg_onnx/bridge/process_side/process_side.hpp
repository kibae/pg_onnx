//
// Created by Kibae Shin on 2023/09/14.
//

#ifndef PG_ONNX_PROCESS_SIDE_HPP
#define PG_ONNX_PROCESS_SIDE_HPP

#include "../../pg_onnx.hpp"

json api_request(
	extension_state_t *state, int16_t type, const json &request_json, const char *post = nullptr, size_t post_length = 0
);

#endif // PG_ONNX_PROCESS_SIDE_HPP
