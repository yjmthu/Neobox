cmake_minimum_required(VERSION 3.15)

find_package(OpenSSL REQUIRED)
find_package(CURL CONFIG REQUIRED)
include_directories(include)

add_library(translate
  src/translate.cpp
)

target_link_libraries(translate PUBLIC
  yjson
  CURL::libcurl
  OpenSSL::SSL
  OpenSSL::Crypto
)

