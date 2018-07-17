cmake_minimum_required(VERSION 3.8)
project(protobuf-l2cache)

set(CMAKE_CXX_STANDARD 11)

if (NOT BOOST_DIR)
    message(FATAL_ERROR "must be set BOOST_DIR")
endif ()

if (NOT PROTOBUF_DIR)
    message(FATAL_ERROR "must be set PROTOBUF_DIR")
endif ()

include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${BOOST_DIR}/include
        ${PROTOBUF_DIR}/include
)

link_directories(
        ${BOOST_DIR}/lib
        ${PROTOBUF_DIR}/lib
)

add_subdirectory(src)
#add_subdirectory(examples)