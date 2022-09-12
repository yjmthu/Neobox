cmake_minimum_required(VERSION 3.10)

include_directories(include)

add_executable(mains
    src/main.cpp
)

add_executable(format
    src/format.cpp
)

add_executable(http
    src/http.cpp
)

target_link_libraries(mains
    yjson
    core
)

target_link_libraries(http
    network
)

add_executable(tests
    src/test.cpp
)

add_executable(translates
  src/translate.cpp
)

target_link_libraries(translates PUBLIC
  translate
)
