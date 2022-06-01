cmake_minimum_required(VERSION 3.10)

include_directories(include)

add_library(core STATIC
    src/netspeedhelper.cpp
    src/sysapi.cpp
    src/timer.cpp
)
