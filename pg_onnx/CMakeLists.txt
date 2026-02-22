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
find_package(CUDAToolkit)
if (NOT CUDAToolkit_FOUND)
    set(CUDAToolkit_ROOT "/usr/local/cuda")
    find_package(CUDAToolkit)
endif ()

if (CUDAToolkit_FOUND)
    if (NOT CMAKE_CUDA_COMPILER)
        find_program(CMAKE_CUDA_COMPILER nvcc PATHS "${CUDAToolkit_BIN_DIR}" NO_DEFAULT_PATH)
    endif ()
    set(REGRESS_FILES ${REGRESS_FILES} 08-cuda)
endif ()


list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
find_package(PostgreSQL REQUIRED)

include_directories(${Boost_INCLUDE_DIR} ${ONNX_RUNTIME_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR} ${PostgreSQL_INCLUDE_DIRS} ${PostgreSQL_SERVER_INCLUDE_DIRS} ../onnxruntime-server/src)
link_directories(${Boost_LIBRARY_DIRS} ${ONNX_RUNTIME_LIBRARY_DIRS} ${OPENSSL_LIBRARY_DIRS})

add_postgresql_extension(
        ${PROJECT_NAME}
        VERSION 1.2.1
        SOURCES

        ${SOURCE_FILES}

        SCRIPTS pg_onnx--1.2.1.sql
        REGRESS ${REGRESS_FILES}
        COMMENT "ONNX Runtime integrated with PostgreSQL. Perform ML inference with data in your database."
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wno-deprecated-register -Wno-register)
target_link_libraries(${PROJECT_NAME} PRIVATE onnxruntime_server_static)

if (APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            INSTALL_RPATH "${ONNX_RUNTIME_LIBRARY_DIRS}"
            BUILD_RPATH "${ONNX_RUNTIME_LIBRARY_DIRS}")
endif ()

enable_testing()
