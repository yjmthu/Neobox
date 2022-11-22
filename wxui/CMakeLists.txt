cmake_minimum_required(VERSION 3.10)

find_package(wxWidgets CONFIG REQUIRED)

add_executable(wxNeobox src/main.cpp)

target_link_libraries(wxNeobox PRIVATE wx::core wx::base)

