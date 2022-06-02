cmake_minimum_required(VERSION 3.5)

project(Neobox VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

if (MSVC)
    add_definitions(-DUNICODE -D_UNICODE -D_CRT_SECURE_NO_WARNINGS=1)
    add_compile_options("/utf-8")
    # add_compile_options("/std:c++latest")
endif ()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# SET(CMAKE_BUILD_TYPE "Debug")
# SET(CMAKE_BUILD_TYPE "Release")

if(WIN32)
    include_directories(
        C:/MSVC2022/OpenSSL/OpenSSL-Win64/include
        C:/MSVC2022/leptonica-1.82.0/include
        C:/MSVC2022/tesseract-5.1.0/include
    )
    link_directories(
        C:/MSVC2022/OpenSSL/OpenSSL-Win64/lib
        C:/MSVC2022/zlib-1.2.12/lib
        C:/MSVC2022/libpng-1.6.35/lib
        C:/MSVC2022/tiff-4.4.0/libtiff/Release
        C:/MSVC2022/leptonica-1.82.0/src/Release
        C:/MSVC2022/tesseract-5.1.0/lib
    )
endif()

set(M_THIRD_PART_LIB_DIR ${CMAKE_SOURCE_DIR}/3rdlib)

include_directories(${M_THIRD_PART_LIB_DIR}/httplib/include)
include_directories(${M_THIRD_PART_LIB_DIR}/YJson/include)
include_directories(${M_THIRD_PART_LIB_DIR}/3rd_qxtglobalshortcut/include)
include_directories(${M_THIRD_PART_LIB_DIR}/httplib)
include_directories(wallpaper/include)
include_directories(translate/include)
include_directories(widgets/include)
include_directories(core/include)

add_subdirectory(${M_THIRD_PART_LIB_DIR}/httplib)
add_subdirectory(${M_THIRD_PART_LIB_DIR}/YJson)
add_subdirectory(${M_THIRD_PART_LIB_DIR}/3rd_qxtglobalshortcut)
add_subdirectory(wallpaper)
add_subdirectory(translate)
add_subdirectory(core)
add_subdirectory(widgets)
add_subdirectory(tests)
