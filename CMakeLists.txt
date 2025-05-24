cmake_minimum_required(VERSION 3.15)

# 限制语言标准为C++98.
set(CMAKE_CXX_STANDARD 98)

# 限制语言标准为C++11.
# set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置项目名称.
project(enpiphany_project)

# 启用调试模式.
set(CMAKE_BUILD_TYPE "Debug")

IF (UNIX)
    MESSAGE(STATUS "enpiphany Run On Unix-like.\n")
    add_definitions(-DEPIPHANY_ON_POSIX)
    set(EPIPHANY_ON_POSIX 1)

    # 将警告视为错误.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
ELSEIF(WIN32)
    MESSAGE(STATUS "enpiphany Run On Win.\n")
    add_definitions(-DEPIPHANY_ON_WIN)
    set(EPIPHANY_ON_WIN 1)

ENDIF ()

# storage core dir.
add_subdirectory(src)

# tests.
add_subdirectory(tests)

# examples.
add_subdirectory(examples/c)