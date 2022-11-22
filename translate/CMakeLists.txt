cmake_minimum_required(VERSION 3.10.1)

find_package(OpenSSL REQUIRED)
find_package(CURL CONFIG REQUIRED)

file(GLOB sources src/*.cpp)

add_library(translate STATIC ${sources})

target_link_libraries(translate PRIVATE CURL::libcurl OpenSSL::SSL OpenSSL::Crypto)
