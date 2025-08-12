include_directories(${PROJECT_SOURCE_DIR}/src/include/common)
include_directories(${PROJECT_SOURCE_DIR}/src/include/util)
include_directories(${PROJECT_SOURCE_DIR}/src/include/bufferpool)
include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src PAGER_SRC_DIR)

add_library(pager STATIC ${PAGER_SRC_DIR})
target_link_libraries(pager PRIVATE common_cache common_mem_manager)