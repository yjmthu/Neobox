cmake_minimum_required(VERSION 3.10.1)

find_package(CURL CONFIG REQUIRED)

file(GLOB sources src/*.cpp)

add_library(network STATIC ${sources})

set(libraries CURL::libcurl)

if (WIN32)
  list(APPEND libraries Wininet)
endif()

target_link_libraries(network PRIVATE ${libraries})
