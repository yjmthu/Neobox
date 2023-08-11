cmake_minimum_required(VERSION 3.10.1)

if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/apikey.cpp")
  add_definitions(-DTRANSLATE_WRONG_USER_KEY)
endif()

file(GLOB source_files src/*.cpp)

set(plugin_name translate_core)
add_library(${plugin_name} STATIC ${source_files})
target_include_directories(${plugin_name} PUBLIC include)

target_link_libraries(${plugin_name} PUBLIC pluginmgr)
