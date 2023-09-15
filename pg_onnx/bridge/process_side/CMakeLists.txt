project(pg_onnx_process_side)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES
        api_client.cpp

        task/task_list_session.cpp
        task/task_create_session.cpp
        task/task_execute_session.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

enable_testing()
