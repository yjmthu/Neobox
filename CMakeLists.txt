cmake_minimum_required(VERSION 3.21.1)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

project(Neobox VERSION 2.0.1 LANGUAGES CXX C)

list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.4.1/msvc2019_64")

add_definitions(-DUNICODE -D_UNICODE)
add_compile_options("/utf-8")

set(CMAKE_INSTALL_PREFIX "install")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# https://blog.csdn.net/10km/article/details/51735432

# if(MSVC)
#   add_compile_options(
#     $<$<CONFIG:>:/MT> #---------|
#     $<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
#     $<$<CONFIG:Release>:/MT> #--|
#   )
# endif()

link_directories(build/pluginmgr)
file(GLOB project_includes */include */*/include)
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

find_package(Qt6 REQUIRED COMPONENTS Widgets)
add_executable(neobox WIN32 main.cpp logo.rc)

target_link_libraries(neobox PRIVATE
  neobox_core
  Qt6::Widgets
)

install(TARGETS neobox
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  PUBLIC_HEADER DESTINATION include
)
