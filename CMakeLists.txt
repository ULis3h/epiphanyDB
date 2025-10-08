cmake_minimum_required(VERSION 3.16)

# Project definition
project(EpiphanyDB
    VERSION 1.0.0
    DESCRIPTION "EpiphanyDB - Multi-modal storage engine based on PostgreSQL 17"
    LANGUAGES C CXX
)

# Set C and C++ standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fPIC")

# Debug flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -DDEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -DDEBUG")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -DNDEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -DNDEBUG")
endif()

# Project paths
set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(POSTGRES_SOURCE_DIR ${PROJECT_ROOT}/postgresql)
set(EPIPHANYDB_SOURCE_DIR ${PROJECT_ROOT}/src)
set(EPIPHANYDB_INCLUDE_DIR ${PROJECT_ROOT}/include)

# Check if PostgreSQL source exists
if(NOT EXISTS ${POSTGRES_SOURCE_DIR})
    message(FATAL_ERROR "PostgreSQL source directory not found: ${POSTGRES_SOURCE_DIR}")
endif()

# Find required dependencies
find_package(PkgConfig REQUIRED)

# OpenSSL
find_package(OpenSSL REQUIRED)

# ICU
pkg_check_modules(ICU REQUIRED icu-uc icu-i18n)

# LibXML2
find_package(LibXml2 REQUIRED)

# LibXSLT
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBXSLT REQUIRED libxslt)

# Zlib
find_package(ZLIB REQUIRED)

# Readline (optional)
find_library(READLINE_LIBRARY readline)
if(READLINE_LIBRARY)
    set(HAVE_READLINE TRUE)
endif()

# Include directories
include_directories(
    ${EPIPHANYDB_INCLUDE_DIR}
    ${POSTGRES_SOURCE_DIR}/src/include
    ${CMAKE_BINARY_DIR}/postgresql/src/include
    ${OPENSSL_INCLUDE_DIR}
    ${ICU_INCLUDE_DIRS}
    ${LIBXML2_INCLUDE_DIR}
    ${LIBXSLT_INCLUDE_DIRS}
)

# Compile definitions
add_definitions(
    -DEPIPHANYDB_VERSION="${PROJECT_VERSION}"
    -DPOSTGRES_VERSION="17.2"
    ${ICU_CFLAGS_OTHER}
    ${LIBXSLT_CFLAGS_OTHER}
)

# Configure PostgreSQL
set(POSTGRES_CONFIGURE_OPTIONS
    --prefix=${CMAKE_BINARY_DIR}/postgresql-install
    --with-openssl
    --with-libxml
    --with-libxslt
    --with-icu
    --enable-debug
    --enable-cassert
    --enable-depend
)

# Custom target to configure PostgreSQL
add_custom_target(configure_postgres
    COMMAND ./configure ${POSTGRES_CONFIGURE_OPTIONS}
    WORKING_DIRECTORY ${POSTGRES_SOURCE_DIR}
    COMMENT "Configuring PostgreSQL..."
)

# Custom target to build PostgreSQL
add_custom_target(build_postgres
    COMMAND make -j${CMAKE_BUILD_PARALLEL_LEVEL}
    WORKING_DIRECTORY ${POSTGRES_SOURCE_DIR}
    DEPENDS configure_postgres
    COMMENT "Building PostgreSQL..."
)

# Custom target to install PostgreSQL
add_custom_target(install_postgres
    COMMAND make install
    WORKING_DIRECTORY ${POSTGRES_SOURCE_DIR}
    DEPENDS build_postgres
    COMMENT "Installing PostgreSQL..."
)

# EpiphanyDB source files
file(GLOB_RECURSE EPIPHANYDB_SOURCES
    "${EPIPHANYDB_SOURCE_DIR}/*.c"
    "${EPIPHANYDB_SOURCE_DIR}/*.cpp"
)

file(GLOB_RECURSE EPIPHANYDB_HEADERS
    "${EPIPHANYDB_INCLUDE_DIR}/*.h"
    "${EPIPHANYDB_SOURCE_DIR}/*.h"
)

# Create EpiphanyDB core library
add_library(epiphanydb_core SHARED
    ${EPIPHANYDB_SOURCES}
    ${EPIPHANYDB_HEADERS}
)

# Link libraries
target_link_libraries(epiphanydb_core
    ${OPENSSL_LIBRARIES}
    ${ICU_LIBRARIES}
    ${LIBXML2_LIBRARIES}
    ${LIBXSLT_LIBRARIES}
    ${ZLIB_LIBRARIES}
)

if(READLINE_LIBRARY)
    target_link_libraries(epiphanydb_core ${READLINE_LIBRARY})
endif()

# Make sure PostgreSQL is built before EpiphanyDB
add_dependencies(epiphanydb_core install_postgres)

# Storage engines
set(STORAGE_ENGINES heap columnar vector timeseries graph)

foreach(ENGINE ${STORAGE_ENGINES})
    file(GLOB ENGINE_SOURCES "${EPIPHANYDB_SOURCE_DIR}/storage/${ENGINE}/*.c")
    if(ENGINE_SOURCES)
        add_library(epiphanydb_${ENGINE} SHARED ${ENGINE_SOURCES})
        target_link_libraries(epiphanydb_${ENGINE} epiphanydb_core)
        add_dependencies(epiphanydb_${ENGINE} install_postgres)
    endif()
endforeach()

# Tests
enable_testing()
file(GLOB TEST_SOURCES "${EPIPHANYDB_SOURCE_DIR}/tests/*.c")
foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} epiphanydb_core)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    add_dependencies(${TEST_NAME} install_postgres)
endforeach()

# Installation
install(TARGETS epiphanydb_core
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY ${EPIPHANYDB_INCLUDE_DIR}/
    DESTINATION include/epiphanydb
    FILES_MATCHING PATTERN "*.h"
)

# Install storage engines
foreach(ENGINE ${STORAGE_ENGINES})
    if(TARGET epiphanydb_${ENGINE})
        install(TARGETS epiphanydb_${ENGINE}
            LIBRARY DESTINATION lib/epiphanydb
        )
    endif()
endforeach()

# Install scripts and configs
install(DIRECTORY scripts/
    DESTINATION bin
    FILES_MATCHING PATTERN "*"
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

install(DIRECTORY config/
    DESTINATION etc/epiphanydb
    FILES_MATCHING PATTERN "*.conf" PATTERN "*.yaml" PATTERN "*.json"
)

# Print configuration summary
message(STATUS "")
message(STATUS "EpiphanyDB Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  PostgreSQL source: ${POSTGRES_SOURCE_DIR}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
message(STATUS "Dependencies:")
message(STATUS "  OpenSSL: ${OPENSSL_VERSION}")
message(STATUS "  ICU: ${ICU_VERSION}")
message(STATUS "  LibXML2: ${LIBXML2_VERSION_STRING}")
message(STATUS "  LibXSLT: Found")
message(STATUS "  Zlib: ${ZLIB_VERSION_STRING}")
message(STATUS "  Readline: ${HAVE_READLINE}")
message(STATUS "")