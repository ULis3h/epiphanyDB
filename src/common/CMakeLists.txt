add_subdirectory(config)
add_subdirectory(cache)
add_subdirectory(mem_manager)

include_directories(${PROJECT_SOURCE_DIR}/src/include/os)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR} COMMON_SRC_DIR)

add_library(common STATIC ${COMMON_SRC_DIR})
target_link_libraries(common PRIVATE common_config 
                                     common_cache 
                                     common_mem_manager)