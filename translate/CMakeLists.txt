cmake_minimum_required(VERSION 3.10.1)

file(GLOB sources src/*.cpp)

add_library(translate STATIC ${sources})

target_link_libraries(translate PRIVATE network)

