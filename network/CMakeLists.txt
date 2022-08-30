cmake_minimum_required(VERSION 3.15)

find_package(CURL CONFIG REQUIRED)
include_directories(include)

add_library(network
   src/httplib.cpp
)

target_link_libraries(network PRIVATE CURL::libcurl)
