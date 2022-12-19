cmake_minimum_required(VERSION 3.10.1)

if (UNIX)
  find_package(CURL CONFIG REQUIRED)
endif()

include_directories(include)
file(GLOB sources src/*.cpp)
add_library(neocore SHARED ${sources})

set(libraries yjson)

if(WIN32)
  list(APPEND libraries Iphlpapi Winhttp)
else()
  list(APPEND libraries CURL::libcurl)
endif()

target_link_libraries(neocore PRIVATE ${libraries})

install(TARGETS neocore
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  PUBLIC_HEADER DESTINATION include
)
