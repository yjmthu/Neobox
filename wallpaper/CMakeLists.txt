cmake_minimum_required(VERSION 3.10)

include_directories(include)

add_library(wallpaper STATIC
    src/wallbase.cpp
    src/wallpaper.cpp
)

target_link_libraries( wallpaper
    Wininet
    network
    yjson
    core
)
