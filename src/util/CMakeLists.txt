include_directories(${PROJECT_SOURCE_DIR}/src/include/os)
include_directories(${PROJECT_SOURCE_DIR}/src/include)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR} UTIL_SRC_DIR)

add_library(util STATIC ${UTIL_SRC_DIR})