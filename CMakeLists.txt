cmake_minimum_required(VERSION 3.5)

project(Neobox VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

if (MSVC)
    # add_definitions(-DUNICODE -D_UNICODE)
    add_compile_options("/utf-8")
endif ()

# SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_BUILD_TYPE "Release")

if(WIN32)
    include_directories(
        C:/MSVC2022/leptonica-1.82.0/include
        C:/MSVC2022/OpenSSL/OpenSSL-Win64/include
        C:/MSVC2022/tesseract/include
    )
endif()

set(M_YJSON_DIR ${CMAKE_SOURCE_DIR}/../YJson)
set(M_THIRD_PART_LIB_DIR ${CMAKE_SOURCE_DIR}/3rdlib)

include_directories(${M_YJSON_DIR}/include)
include_directories(${M_THIRD_PART_LIB_DIR}/3rd_qxtglobalshortcut/include)
include_directories(${M_THIRD_PART_LIB_DIR}/httplib)
include_directories(wallpaper/include)
include_directories(translate/include)
include_directories(widgets/include)
include_directories(core/include)

add_subdirectory(${M_YJSON_DIR} yjson)
add_subdirectory(${M_THIRD_PART_LIB_DIR}/3rd_qxtglobalshortcut 3rd_qxtglobalshortcut)
add_subdirectory(wallpaper)
add_subdirectory(translate)
add_subdirectory(core)
add_subdirectory(widgets)
add_subdirectory(tests)

# add_executable(Neobox 
#     master.cpp
#     ${COLYOO_QT_RESOURCES}
# )

# target_link_libraries(Neobox PUBLIC
#     wallpaper
#     core
#     translate
#     widgets
# )
