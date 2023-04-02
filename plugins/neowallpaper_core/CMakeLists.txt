cmake_minimum_required(VERSION 3.10.1)

file(GLOB sources src/*.cpp)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(wallpaper_core STATIC ${sources})

target_link_libraries(wallpaper_core PRIVATE pluginmgr)
