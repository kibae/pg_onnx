cmake_minimum_required(VERSION 3.22.0)
project(pg_onnx)

set(CMAKE_CXX_STANDARD 17)

set(NO_ONNXRUNTIME_SERVER_OPENSSL TRUE)
set(NO_ONNXRUNTIME_SERVER_TEST TRUE)
set(NO_ONNXRUNTIME_SERVER_INSTALL TRUE)
add_subdirectory(onnxruntime-server)

add_subdirectory(pg_onnx)

enable_testing()
