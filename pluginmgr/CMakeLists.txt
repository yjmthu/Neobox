cmake_minimum_required(VERSION 3.10.1)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)
file(GLOB source_files src/*.cpp)

set(system_shared_files Shell32)
set(qt_shared_files Qt6::Core Qt6::Widgets Qt6::Gui)
set(pliugin_libfiles yjson)

add_library(pluginmgr STATIC ${source_files})

target_link_libraries(pluginmgr PRIVATE
  ${pliugin_libfiles}
  ${qt_shared_files}
  ${system_shared_files}
)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  # message(STATUS "============deBUG=================")
  # target_link_libraries(pluginmgr PRIVATE neospeedboxplg)
else()
endif()