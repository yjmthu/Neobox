cmake_minimum_required(VERSION 3.21.1)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

project(Neobox VERSION 2.0.1 LANGUAGES CXX C)

set(VCPKG_ROOT "${CMAKE_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE PATH "build")
list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.4.1/msvc2019_64")

add_definitions(-DUNICODE -D_UNICODE)
add_compile_options("/utf-8")

set(CMAKE_INSTALL_PREFIX "install")
set(VCPKG_TARGET_TRIPLET "x86-windows-static")
set(CMAKE_TOOLCHAIN_FILE ${VCPKG_ROOT})


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB project_includes */include)
file(GLOB thrd_includes 3rdlib/*/include)
file(GLOB project_subdirs */CMakeLists.txt)
foreach(item ${project_includes})
  message(STATUS "include dir is ${item}")
endforeach()
include_directories(${project_includes})
include_directories(../YJson/include)
add_subdirectory(../YJson yjson)

foreach(item ${project_subdirs})
  string(REGEX REPLACE "/CMakeLists.txt" "" subdir ${item})
  get_filename_component(subdir ${subdir} NAME)
  add_subdirectory(${subdir})
endforeach()

add_executable(hi hello.cpp)

install(TARGETS Neobox
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  PUBLIC_HEADER DESTINATION include
)

