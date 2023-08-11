cmake_minimum_required(VERSION 3.10.1)

file(GLOB source_files src/*.cpp include/*.hpp)

# set(system_shared_files Shell32)
set(qt_shared_files Qt6::Core Qt6::Widgets Qt6::Gui)
set(pliugin_libfiles translate_core pluginmgr)

set(plugin_name neotranslateplg)
add_library(${plugin_name} SHARED
  ${source_files}
)
target_include_directories(${plugin_name} PRIVATE include)

set(resource_files icons/*.png)
load_resources(${plugin_name} resource_files)

target_link_libraries(${plugin_name} PUBLIC
  ${pliugin_libfiles}
  ${qt_shared_files}
  # ${system_shared_files}
)

install_plugin(${plugin_name})
