cmake_minimum_required(VERSION 3.10)

include_directories(include)

add_executable(test_sys
    src/main.cpp
)

add_executable(format
    src/format.cpp
)

add_executable(http
    src/http.cpp
)

target_link_libraries(test_sys
    yjson
    core
)

target_link_libraries(http
    network
)

add_executable(test_main
    src/test.cpp
)
