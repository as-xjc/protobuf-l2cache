cmake_minimum_required(VERSION 3.8)
project(protobuf-l2cache)

set(CMAKE_CXX_STANDARD 17)

if (NOT PROTOBUF_DIR)
    message(FATAL_ERROR "must be set PROTOBUF_DIR")
endif ()

include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${PROTOBUF_DIR}/include
)

link_directories(
        ${PROTOBUF_DIR}/lib
)

add_subdirectory(src)
#add_subdirectory(examples)