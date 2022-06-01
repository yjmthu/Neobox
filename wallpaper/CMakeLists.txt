cmake_minimum_required(VERSION 3.10)

include_directories(include)

add_library(wallpaper STATIC
    src/apiclass.cpp
    src/wallpaper.cpp
)

target_link_libraries( wallpaper
    yjson
    core
    ssl
    crypto
)
