cmake_minimum_required(VERSION 3.21.2)

project(Neobox VERSION 2.3.5 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INSTALL_PREFIX "install")

string(TIMESTAMP COMPILE_TIME %Y-%m-%d)
set(NEOBOX_BUILD_TIME ${COMPILE_TIME})

# list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.4.2/msvc2019_64")

add_definitions(-DUNICODE -D_UNICODE)
add_compile_options("/utf-8")

link_directories(build/pluginmgr)
include_directories(pluginmgr/include)
include_directories(thirdlib/YJson/include)

add_subdirectory(thirdlib/YJson yjson)
add_subdirectory(pluginmgr)

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
