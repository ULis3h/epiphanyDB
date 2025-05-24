include_directories(${PROJECT_SOURCE_DIR}/include)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR} EXAMPLE_C_SRC_DIR)

# set target.
add_executable(example_c_base
# sources.
${EXAMPLE_C_SRC_DIR}
)
target_link_libraries(example_c_base epiphany)