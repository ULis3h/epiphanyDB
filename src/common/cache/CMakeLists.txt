include_directories(${PROJECT_SOURCE_DIR}/include/common)
include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src CACHE_SRC_DIR)

# Add source groups for header files
file(GLOB_RECURSE CACHE_INCLUDE_DIR  ${CMAKE_CURRENT_LIST_DIR}/include/*.hpp )

# set target.
add_library(common_cache STATIC
# headers. 
${CACHE_INCLUDE_DIR}

# sources.
${CACHE_SRC_DIR}
)

# Add reference files to the project
target_include_directories(
    common_cache PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/include/common
)



# compiles configs.