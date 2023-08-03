cmake_minimum_required(VERSION 3.10.1)

if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/apikey.h")
  add_definitions(-DTRANSLATE_USE_USER_KEY)
endif()

file(GLOB source_files src/*.cpp)

add_library(translate_core STATIC ${source_files})

target_link_libraries(translate_core PUBLIC pluginmgr)
