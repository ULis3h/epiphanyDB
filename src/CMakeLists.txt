add_subdirectory(os)
add_subdirectory(common)
add_subdirectory(bufferpool)

include_directories(include)
include_directories(include/os)
include_directories(include/common)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR} EPIPHANY_SRC_DIR)

# comb.
add_library(epiphany STATIC ${EPIPHANY_SRC_DIR})


# Add reference files to the project
target_include_directories(
    epiphany PUBLIC 
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(epiphany PRIVATE common os)