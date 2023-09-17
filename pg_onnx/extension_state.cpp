//
// Created by Kibae Shin on 2023/09/14.
//

#include "pg_onnx.hpp"

// cache per process
extension_state_t *state = nullptr;

extension_state_t *extension_state() {
	if (state != nullptr) {
		return state;
	}
	bool found = false;

	auto size = sizeof(extension_state_t);
	RequestAddinShmemSpace(size);
	state = (extension_state_t *)ShmemInitStruct("pg_onnx_state", size, &found);
	if (!found) {
		memset(state, 0, size);
		state->pid = -1;
		state->port = 0;
		state->config_num_threads = 4;
		state->config_onnxruntime_server_port = -1;
	}

	return state;
}

bool is_onnxruntime_server_remote(const extension_state_t *st) {
	return st->config_onnxruntime_server_port > 0 && strlen(st->config_onnxruntime_server_hostname) > 0;
}
