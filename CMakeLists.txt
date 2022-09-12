cmake_minimum_required(VERSION 3.5)

project(NEOBOX VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_definitions(-DUNICODE -D_UNICODE -D_CRT_SECURE_NO_WARNINGS=1)
    add_compile_options("/utf-8")
endif ()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(M_THIRD_PART_LIB_DIR ${CMAKE_SOURCE_DIR}/3rdlib)

include_directories(${M_THIRD_PART_LIB_DIR}/YJson/include)
include_directories(wallpaper/include)
include_directories(widgets/include)
include_directories(core/include)
include_directories(network/include)
include_directories(translate/include)

add_subdirectory(${M_THIRD_PART_LIB_DIR}/YJson)
add_subdirectory(wallpaper)
add_subdirectory(core)
add_subdirectory(widgets)
add_subdirectory(network)
add_subdirectory(translate)
add_subdirectory(tests)
