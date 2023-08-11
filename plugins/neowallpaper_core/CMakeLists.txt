cmake_minimum_required(VERSION 3.10.1)

file(GLOB sources src/*.cpp)

set(plugin_name wallpaper_core)
add_library(${plugin_name} STATIC ${sources})
target_include_directories(${plugin_name} PUBLIC include)

target_link_libraries(${plugin_name} PRIVATE pluginmgr)
