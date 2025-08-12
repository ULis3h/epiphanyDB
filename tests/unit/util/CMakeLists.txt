include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/src/include/os)
include_directories(${PROJECT_SOURCE_DIR}/src/include/util)
include_directories(${PROJECT_SOURCE_DIR}/src/include/common)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR} SRC_DIR)

# set target.
add_executable(util_test
        # sources.
        ${SRC_DIR}
)

target_link_libraries(util_test epiphany)

# compiles configs.

IF(UNIX OR APPLE)
    # Treat warnings as errors.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
ELSEIF(WIN32)
    IF(MSVC)
        # Treat warnings as errors.
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
        target_link_libraries(util_test common os)
        target_compile_options(util_test PRIVATE /utf-8)
    ENDIF() # WIN32
ENDIF ()