cmake_minimum_required(VERSION 3.10.1)

find_package(minizip CONFIG REQUIRED)
file(GLOB sources src/*.cpp)

foreach(item ${sources})
  string(REGEX MATCH "src/.*" target_name ${item})
  string(REGEX REPLACE "src/" "test_" target_name ${target_name})
  string(REGEX REPLACE "/|\\." "_" target_name ${target_name})
  add_executable(${target_name} ${item})
  target_link_libraries(${target_name} yjson core network translate_core minizip::minizip)
endforeach()