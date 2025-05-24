include_directories(${PROJECT_SOURCE_DIR}/src/include/os)
include_directories(${PROJECT_SOURCE_DIR}/src/include/common)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR} OS_SRC_DIR)

add_library(os STATIC ${OS_SRC_DIR}) 
target_link_libraries(os pthread)