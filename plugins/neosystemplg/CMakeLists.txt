cmake_minimum_required(VERSION 3.10.1)

file(GLOB source_files src/*.cpp)

if(WIN32)
  set(system_shared_files Shell32 PowrProf)
else()
  set(system_shared_files "")
endif()

set(qt_shared_files Qt6::Core Qt6::Widgets Qt6::Gui)
set(pliugin_libfiles pluginmgr)
set(plugin_name neosystemplg)

add_library(${plugin_name} SHARED ${source_files})
target_include_directories(${plugin_name} PRIVATE include)

set(resource_files icons/*)
load_resources(${plugin_name} resource_files)

target_link_libraries(${plugin_name} PRIVATE
  ${pliugin_libfiles}
  ${qt_shared_files}
  ${system_shared_files}
)

install_plugin(${plugin_name})
