cmake_minimum_required(VERSION 3.21)

add_definitions(-DUNICODE -D_UNICODE)

project(UPDATE C CXX)

set(SRC_LIST update.cpp yjson.cpp zip.c)

set(UPDATE_BINARY_DIR ./build)

set(UPDATE_SOURCE_DIR ./)

message(STATUS "THIS IS BINARY DIR" ${UPDATE_BINARY_DIR})

message(STATUS "THIS IS SOURCE DIR" ${UPDATE_SOURCE_DIR})

set(CMAKE_INCLUDE_CURRENT_DIR ON) #当前include 打开

# include_directory(.) #  or include_directory(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(update ${SRC_LIST} )
