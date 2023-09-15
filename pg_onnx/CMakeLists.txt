cmake_minimum_required(VERSION 3.22.0)
project(pg_onnx)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
        pg_onnx.cpp
        extension_state.cpp
        pg_background_worker.cpp
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
find_package(PostgreSQL REQUIRED)

include_directories(${Boost_INCLUDE_DIR} ${ONNX_RUNTIME_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} ${PostgreSQL_SERVER_INCLUDE_DIRS} ../onnxruntime-server/src)

add_subdirectory(bridge/bgworker_side)
add_subdirectory(bridge/process_side)

add_postgresql_extension(
        ${PROJECT_NAME}
        VERSION 1.0
        SOURCES

        ${SOURCE_FILES}

        SCRIPTS pg_onnx--1.0.sql
        REGRESS 01-create-model 04-session 09-drop-model)


target_link_libraries(${PROJECT_NAME} PRIVATE onnxruntime_server_static pg_onnx_bgworker_side pg_onnx_process_side)

enable_testing()
