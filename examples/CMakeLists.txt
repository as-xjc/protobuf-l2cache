add_executable(example main.cpp
        test.pb.cc)

include_directories(${PROJECT_SOURCE_DIR}/src)

target_link_libraries(example p2cache)