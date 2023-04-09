cmake_minimum_required(VERSION 3.10.1)

file(GLOB source_files src/*.cpp)

set(qt_shared_files Qt6::Core Qt6::Widgets Qt6::Gui)
set(pliugin_libfiles pluginmgr yjson)

add_library(weather SHARED ${source_files})

target_link_libraries(weather PRIVATE
  ${pliugin_libfiles}
  ${qt_shared_files}
)
