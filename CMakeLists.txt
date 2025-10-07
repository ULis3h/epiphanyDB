cmake_minimum_required(VERSION 3.16)

# 项目配置
project(EpiphanyDB
    VERSION 1.0.0
    DESCRIPTION "超融合多模态数据库 - 基于 PostgreSQL 17"
    LANGUAGES C CXX
)

# 设置 C/C++ 标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 编译选项
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# 项目路径
set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(POSTGRES_SOURCE_DIR ${PROJECT_ROOT}/postgresql)
set(EPIPHANYDB_SOURCE_DIR ${PROJECT_ROOT}/src)
set(EPIPHANYDB_INCLUDE_DIR ${PROJECT_ROOT}/include)

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 包含目录
include_directories(
    ${EPIPHANYDB_INCLUDE_DIR}
    ${POSTGRES_SOURCE_DIR}/src/include
    ${CMAKE_BINARY_DIR}/postgresql/src/include
)

# 查找依赖包
find_package(PkgConfig REQUIRED)

# OpenSSL
find_package(OpenSSL REQUIRED)
if(OpenSSL_FOUND)
    message(STATUS "Found OpenSSL: ${OPENSSL_VERSION}")
endif()

# ICU
pkg_check_modules(ICU REQUIRED icu-uc icu-i18n)

# LibXML2
find_package(LibXml2 REQUIRED)

# LibXSLT
pkg_check_modules(LIBXSLT REQUIRED libxslt)

# LLVM (可选)
find_package(LLVM QUIET)
if(LLVM_FOUND)
    message(STATUS "Found LLVM: ${LLVM_PACKAGE_VERSION}")
    add_definitions(-DHAVE_LLVM)
endif()

# Python (可选)
find_package(Python3 COMPONENTS Interpreter Development QUIET)
if(Python3_FOUND)
    message(STATUS "Found Python: ${Python3_VERSION}")
    add_definitions(-DHAVE_PYTHON)
endif()

# 编译选项
add_compile_options(
    -Wall
    -Wextra
    -Wno-unused-parameter
    -Wno-sign-compare
    -fPIC
)

# 调试模式额外选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(
        -fsanitize=address
        -fsanitize=undefined
        -fno-omit-frame-pointer
    )
    add_link_options(
        -fsanitize=address
        -fsanitize=undefined
    )
endif()

# PostgreSQL 配置检查
if(NOT EXISTS ${POSTGRES_SOURCE_DIR}/configure.ac)
    message(FATAL_ERROR "PostgreSQL 源码不存在或不完整: ${POSTGRES_SOURCE_DIR}")
endif()

# 自定义目标：配置 PostgreSQL
add_custom_target(configure_postgres
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/build_config.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "配置 PostgreSQL..."
)

# 自定义目标：构建 PostgreSQL
add_custom_target(build_postgres
    COMMAND make -C ${POSTGRES_SOURCE_DIR} -j${CMAKE_BUILD_PARALLEL_LEVEL}
    DEPENDS configure_postgres
    COMMENT "构建 PostgreSQL..."
)

# 自定义目标：安装 PostgreSQL
add_custom_target(install_postgres
    COMMAND make -C ${POSTGRES_SOURCE_DIR} install
    DEPENDS build_postgres
    COMMENT "安装 PostgreSQL..."
)

# EpiphanyDB 核心库
if(EXISTS ${EPIPHANYDB_SOURCE_DIR})
    # 收集源文件
    file(GLOB_RECURSE EPIPHANYDB_SOURCES
        ${EPIPHANYDB_SOURCE_DIR}/*.c
        ${EPIPHANYDB_SOURCE_DIR}/*.cpp
    )
    
    file(GLOB_RECURSE EPIPHANYDB_HEADERS
        ${EPIPHANYDB_INCLUDE_DIR}/*.h
        ${EPIPHANYDB_INCLUDE_DIR}/*.hpp
    )
    
    # 创建核心库
    add_library(epiphanydb_core SHARED
        ${EPIPHANYDB_SOURCES}
        ${EPIPHANYDB_HEADERS}
    )
    
    # 链接库
    target_link_libraries(epiphanydb_core
        ${OPENSSL_LIBRARIES}
        ${ICU_LIBRARIES}
        ${LIBXML2_LIBRARIES}
        ${LIBXSLT_LIBRARIES}
    )
    
    # 包含目录
    target_include_directories(epiphanydb_core PRIVATE
        ${OPENSSL_INCLUDE_DIR}
        ${ICU_INCLUDE_DIRS}
        ${LIBXML2_INCLUDE_DIR}
        ${LIBXSLT_INCLUDE_DIRS}
    )
    
    # 编译定义
    target_compile_definitions(epiphanydb_core PRIVATE
        -DEPIPHANYDB_VERSION="${PROJECT_VERSION}"
        -DPOSTGRES_VERSION="17.2"
    )
    
    # 依赖 PostgreSQL
    add_dependencies(epiphanydb_core install_postgres)
endif()

# 存储引擎扩展
set(STORAGE_ENGINES
    heap_engine
    columnar_engine
    vector_engine
    timeseries_engine
    graph_engine
)

foreach(ENGINE ${STORAGE_ENGINES})
    set(ENGINE_DIR ${EPIPHANYDB_SOURCE_DIR}/storage/${ENGINE})
    if(EXISTS ${ENGINE_DIR})
        file(GLOB ENGINE_SOURCES ${ENGINE_DIR}/*.c ${ENGINE_DIR}/*.cpp)
        if(ENGINE_SOURCES)
            add_library(${ENGINE} SHARED ${ENGINE_SOURCES})
            target_link_libraries(${ENGINE} epiphanydb_core)
            add_dependencies(${ENGINE} epiphanydb_core)
        endif()
    endif()
endforeach()

# 测试配置
enable_testing()

# 单元测试
if(EXISTS ${PROJECT_ROOT}/tests)
    file(GLOB_RECURSE TEST_SOURCES ${PROJECT_ROOT}/tests/*.c)
    foreach(TEST_SOURCE ${TEST_SOURCES})
        get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
        add_executable(${TEST_NAME} ${TEST_SOURCE})
        target_link_libraries(${TEST_NAME} epiphanydb_core)
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
        add_dependencies(${TEST_NAME} epiphanydb_core)
    endforeach()
endif()

# 性能测试
add_custom_target(benchmark
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/epiphanydb/bin/pgbench -i -s 10 postgres
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/epiphanydb/bin/pgbench -c 10 -j 2 -t 1000 postgres
    DEPENDS install_postgres
    COMMENT "运行性能测试..."
)

# 回归测试
add_custom_target(regression_test
    COMMAND make -C ${POSTGRES_SOURCE_DIR} check
    DEPENDS build_postgres
    COMMENT "运行回归测试..."
)

# 代码覆盖率
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    find_program(GCOV_PATH gcov)
    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)
    
    if(GCOV_PATH AND LCOV_PATH AND GENHTML_PATH)
        add_compile_options(--coverage)
        add_link_options(--coverage)
        
        add_custom_target(coverage
            COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.info
            COMMAND ${LCOV_PATH} --remove coverage.info '/usr/*' --output-file coverage.info
            COMMAND ${LCOV_PATH} --list coverage.info
            COMMAND ${GENHTML_PATH} -o coverage coverage.info
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "生成代码覆盖率报告..."
        )
    endif()
endif()

# 文档生成
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    
    if(EXISTS ${DOXYGEN_IN})
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        
        add_custom_target(docs
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "生成 API 文档..."
            VERBATIM
        )
    endif()
endif()

# 安装配置
install(TARGETS epiphanydb_core
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

# 安装头文件
install(DIRECTORY ${EPIPHANYDB_INCLUDE_DIR}/
    DESTINATION include/epiphanydb
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# 安装存储引擎
foreach(ENGINE ${STORAGE_ENGINES})
    if(TARGET ${ENGINE})
        install(TARGETS ${ENGINE}
            LIBRARY DESTINATION lib/epiphanydb/engines
        )
    endif()
endforeach()

# 安装脚本
install(PROGRAMS
    ${CMAKE_CURRENT_SOURCE_DIR}/start_epiphanydb.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/stop_epiphanydb.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/epiphanydb_env.sh
    DESTINATION bin
)

# 安装配置文件
install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/docs/POSTGRES_INTEGRATION.md
    ${CMAKE_CURRENT_SOURCE_DIR}/docs/MULTI_STORAGE_ENGINE_DESIGN.md
    ${CMAKE_CURRENT_SOURCE_DIR}/docs/HYPERCONVERGED_ARCHITECTURE.md
    DESTINATION share/doc/epiphanydb
)

# CPack 配置
set(CPACK_PACKAGE_NAME "EpiphanyDB")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VENDOR "EpiphanyDB Team")
set(CPACK_PACKAGE_CONTACT "team@epiphanydb.org")

set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_GENERATOR "TGZ;DEB;RPM")

set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libssl3, libxml2, libxslt1.1, libicu70")
set(CPACK_RPM_PACKAGE_REQUIRES "glibc, openssl-libs, libxml2, libxslt, libicu")

include(CPack)

# 显示配置信息
message(STATUS "")
message(STATUS "EpiphanyDB 配置摘要:")
message(STATUS "  版本: ${PROJECT_VERSION}")
message(STATUS "  构建类型: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C 编译器: ${CMAKE_C_COMPILER}")
message(STATUS "  C++ 编译器: ${CMAKE_CXX_COMPILER}")
message(STATUS "  安装前缀: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  PostgreSQL 源码: ${POSTGRES_SOURCE_DIR}")
message(STATUS "")
message(STATUS "依赖库:")
message(STATUS "  OpenSSL: ${OPENSSL_VERSION}")
message(STATUS "  ICU: ${ICU_VERSION}")
message(STATUS "  LibXML2: ${LIBXML2_VERSION_STRING}")
if(LLVM_FOUND)
    message(STATUS "  LLVM: ${LLVM_PACKAGE_VERSION}")
endif()
if(Python3_FOUND)
    message(STATUS "  Python: ${Python3_VERSION}")
endif()
message(STATUS "")
message(STATUS "构建目标:")
message(STATUS "  make all          - 构建所有目标")
message(STATUS "  make install      - 安装 EpiphanyDB")
message(STATUS "  make test         - 运行测试")
message(STATUS "  make benchmark    - 运行性能测试")
message(STATUS "  make docs         - 生成文档")
message(STATUS "  make package      - 创建安装包")
message(STATUS "")