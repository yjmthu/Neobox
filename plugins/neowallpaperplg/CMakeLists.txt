cmake_minimum_required(VERSION 3.10.1)

file(GLOB source_files src/*.cpp)

if(WIN32)
  set(system_shared_files Shell32)
else()
  set(system_shared_files "")
endif()
set(qt_shared_files Qt6::Core Qt6::Widgets Qt6::Gui)
set(pliugin_libfiles wallpaper_core pluginmgr)

set(plugin_name neowallpaperplg)
add_library(${plugin_name} SHARED ${source_files})

target_link_libraries(${plugin_name} PRIVATE
  ${pliugin_libfiles}
  ${qt_shared_files}
  ${system_shared_files}
)

install_plugin(${plugin_name})
