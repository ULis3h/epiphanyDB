include_directories(${PROJECT_SOURCE_DIR}/src/include/common)
include_directories(${PROJECT_SOURCE_DIR}/src/include/os)
include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src MEM_MANAGER_SRC_DIR)

# Add source groups for header files
file(GLOB_RECURSE MEM_MANAGER_INC_DIR  ${CMAKE_CURRENT_LIST_DIR}/include/*.hpp )

# set target.
add_library(common_mem_manager STATIC
# headers. 
${MEM_MANAGER_INC_DIR}

# sources.
${MEM_MANAGER_SRC_DIR}
)

# Add reference files to the project
target_include_directories(
    common_mem_manager PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/include/common
)



# compiles configs.