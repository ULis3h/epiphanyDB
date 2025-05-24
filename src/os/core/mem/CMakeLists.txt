
# Determine bit width.
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_definitions(-DMEM_MANAGER_64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    add_definitions(-DMEM_MANAGER_32)
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
    add_definitions(-DMEM_MANAGER_64)
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "i386" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "x86")
    add_definitions(-DMEM_MANAGER_32)   
else()
    add_definitions(-DMEM_MANAGER_32)
endif()

include_directories(${PROJECT_SOURCE_DIR}/src/include/os)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR} OS_SRC_DIR)