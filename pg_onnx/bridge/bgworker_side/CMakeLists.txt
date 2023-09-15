project(pg_onnx_bgworker_side)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES
        bgworker.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

enable_testing()
