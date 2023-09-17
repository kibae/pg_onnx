cmake_minimum_required(VERSION 3.22.0)
project(pg_onnx)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
        pg_onnx.cpp
        extension_state.cpp
        pg_background_worker.cpp

        bridge/bgworker_side/bgworker.cpp

        bridge/process_side/api_client.cpp
        bridge/process_side/task/task_list_session.cpp
        bridge/process_side/task/task_describe_session.cpp
        bridge/process_side/task/task_create_session.cpp
        bridge/process_side/task/task_execute_session.cpp
        bridge/process_side/task/task_destroy_session.cpp
)

set(REGRESS_FILES
        01-create-model
        04-session
        05-trigger
        09-drop-model
)

# CUDA
find_package(CUDA)
if (NOT CUDA_FOUND)
    set(CUDA_SDK_ROOT_DIR "/usr/local/cuda")
    find_package(CUDA)
endif ()

if (CUDA_FOUND)
    set(REGRESS_FILES ${REGRESS_FILES} 08-cuda)
endif ()


list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
find_package(PostgreSQL REQUIRED)

include_directories(${Boost_INCLUDE_DIR} ${ONNX_RUNTIME_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} ${PostgreSQL_SERVER_INCLUDE_DIRS} ../onnxruntime-server/src)

add_postgresql_extension(
        ${PROJECT_NAME}
        VERSION 1.1.0
        SOURCES

        ${SOURCE_FILES}

        SCRIPTS pg_onnx--1.2.0.sql
        REGRESS ${REGRESS_FILES}
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wno-deprecated-register -Wno-register)
target_link_libraries(${PROJECT_NAME} PRIVATE onnxruntime_server_static)

enable_testing()
