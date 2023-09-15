//
// Created by Kibae Shin on 2023/09/14.
//

#ifndef PG_ONNX_BGWORKER_SIDE_HPP
#define PG_ONNX_BGWORKER_SIDE_HPP

#include "../../pg_onnx.hpp"
#include "logger_pipe.hpp"

#include "utils/aixlog.hpp"

int bgworker_init(extension_state_t *state, volatile sig_atomic_t *terminated);
std::string model_bin_getter(const std::string &model_name, const std::string &model_version);

#endif // PG_ONNX_BGWORKER_SIDE_HPP
