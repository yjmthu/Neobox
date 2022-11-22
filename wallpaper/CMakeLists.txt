cmake_minimum_required(VERSION 3.10.1)

file(GLOB sources src/*.cpp)

add_library(wallpaper STATIC ${sources})

target_link_libraries(wallpaper network yjson core)