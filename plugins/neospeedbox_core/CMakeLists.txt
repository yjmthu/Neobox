cmake_minimum_required(VERSION 3.10.1)

include_directories(include)
file(GLOB sources src/*.cpp)

set(plugin_name neospeedbox_core)
add_library(${plugin_name} STATIC ${sources})
target_include_directories(${plugin_name} PUBLIC include)

if(WIN32)
  target_link_libraries(${plugin_name} PRIVATE Iphlpapi)
endif()
