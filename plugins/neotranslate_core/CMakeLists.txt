cmake_minimum_required(VERSION 3.10.1)

file(GLOB source_files src/*.cpp)

add_library(translate_core STATIC ${source_files})

target_link_libraries(translate_core PUBLIC pluginmgr)
