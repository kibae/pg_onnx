//
// Created by Kibae Shin on 2023/09/14.
//

#ifndef PG_ONNX_PROCESS_SIDE_HPP
#define PG_ONNX_PROCESS_SIDE_HPP

#include "../../pg_onnx.hpp"

class post_data {
  protected:
	size_t _content_length;

  public:
	post_data(size_t content_length = -1) : _content_length(content_length) {
	}

	virtual bool eof() = 0;
	virtual size_t read(char *buffer, size_t size) = 0;

	size_t content_length() {
		return _content_length;
	}
};

json api_request(
	extension_state_t *state, int16_t type, const json &request_json, const std::shared_ptr<post_data> &post = nullptr
);

#endif // PG_ONNX_PROCESS_SIDE_HPP
