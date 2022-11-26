cmake_minimum_required(VERSION 3.10.1)

include_directories(include)
file(GLOB sources src/*.cpp)
add_library(neospeedbox_core STATIC ${sources})

if(WIN32)
  target_link_libraries(neospeedbox_core PRIVATE Iphlpapi)
endif()
