add_executable(enpiphanyclient)
MESSAGE("Begin to build " enpiphanyclient)

file(GLOB_RECURSE ALL_SRC *.cpp)

target_sources(enpiphanyclient PRIVATE ${ALL_SRC})

