cmake_minimum_required(VERSION 3.21.2)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INSTALL_PREFIX "install")

# https://zhuanlan.zhihu.com/p/429849408
# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:DebugDLL>")

project(Neobox VERSION 2.3.3 LANGUAGES CXX C)

string(TIMESTAMP COMPILE_TIME %Y-%m-%d)
set(NEOBOX_BUILD_TIME ${COMPILE_TIME})

# list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.4.2/msvc2019_64")

add_definitions(-DUNICODE -D_UNICODE)
add_compile_options("/utf-8")

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

target_link_libraries(neobox PUBLIC
  pluginmgr
  yjson
  Qt6::Widgets
)

install(TARGETS neobox
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  PUBLIC_HEADER DESTINATION include
)
