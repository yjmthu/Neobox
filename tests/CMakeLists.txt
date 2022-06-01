cmake_minimum_required(VERSION 3.10)

include_directories(include)

add_executable(test_sys
    src/main.cpp
)

target_link_libraries(
    test_sys
    yjson
    core
)

add_executable(test_main
    src/test.cpp
)