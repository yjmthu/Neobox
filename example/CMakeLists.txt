cmake_minimum_required(VERSION 3.10.1)

file(GLOB sources src/*.cpp)

foreach(item ${sources})
  string(REGEX MATCH "src/.*" target_name ${item})
  string(REGEX REPLACE "src/" "test_" target_name ${target_name})
  string(REGEX REPLACE "/|\\." "_" target_name ${target_name})
  add_executable(${target_name} ${item})
  target_link_libraries(${target_name} pluginmgr translate_core)
endforeach()

if(UNIX)
  add_executable(x11_test src/x11_test.cc)
  target_link_libraries(x11_test PUBLIC X11)
endif()
