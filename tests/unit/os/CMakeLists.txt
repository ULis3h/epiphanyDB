include_directories(${PROJECT_SOURCE_DIR}/src/include/os)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR} SRC_DIR)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/os TEST_SRC_DIR)

# set target.
add_executable(os_test
# sources.
${SRC_DIR}
${TEST_SRC_DIR}
)

target_link_libraries(os_test epiphany)

# compiles configs.

IF(UNIX OR APPLE)
    # Treat warnings as errors.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
ELSEIF(WIN32)
    IF(MSVC)
        # Treat warnings as errors.
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
        target_link_libraries(os_test common os)
        target_compile_options(os_test PRIVATE /utf-8)
    ENDIF() # WIN32
ENDIF ()