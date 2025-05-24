include_directories(${PROJECT_SOURCE_DIR}/src/include/os)
include_directories(${PROJECT_SOURCE_DIR}/src/include/common)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR} SRC_DIR)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/config TEST_CONFIG_SRC_DIR)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/cache TEST_CACHE_SRC_DIR)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/mem_manager 
TEST_MEM_MANAGER_SRC_DIR)

# set target.
add_executable(common_test
# sources.
${SRC_DIR}
${TEST_CONFIG_SRC_DIR}
${TEST_CACHE_SRC_DIR}
${TEST_MEM_MANAGER_SRC_DIR}
)
target_link_libraries(common_test epiphany)

# compiles configs.

IF(UNIX OR APPLE)
    # Treat warnings as errors.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
ELSEIF(WIN32)
    IF(MSVC)
        # Treat warnings as errors.
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
        target_link_libraries(common_test common os)
        target_compile_options(common_test PRIVATE /utf-8)
    ENDIF() # WIN32
ENDIF ()