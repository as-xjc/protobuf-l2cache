add_library(p2cache
        cache.cpp
        generall1cache.cpp
        )
target_link_libraries(p2cache libprotobuf.a pthread)

install(TARGETS p2cache ARCHIVE DESTINATION lib)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/p2cache DESTINATION include)