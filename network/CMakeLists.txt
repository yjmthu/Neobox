cmake_minimum_required(VERSION 3.10.1)

if (UNIX)
  find_package(CURL CONFIG REQUIRED)
endif()

file(GLOB sources src/*.cpp)

add_library(network STATIC ${sources})


if (WIN32)
  set(libraries Winhttp)
else()
  set(libraries CURL::libcurl)
endif()

target_link_libraries(network PRIVATE ${libraries})
