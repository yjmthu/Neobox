cmake_minimum_required(VERSION 3.21)

project(DIR2JS CXX)

set(SRC_LIST main.cpp yjson.cpp)

set(DIR2JS_BINARY_DIR ./build)

set(DIR2JS_SOURCE_DIR ./)

message(STATUS "THIS IS BINARY DIR" ${DIR2JS_BINARY_DIR})

message(STATUS "THIS IS SOURCE DIR" ${DIR2JS_SOURCE_DIR})

set(CMAKE_INCLUDE_CURRENT_DIR ON) #当前include 打开

# include_directory(.) #  or include_directory(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(dir2js ${SRC_LIST} )
