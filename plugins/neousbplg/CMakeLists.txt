cmake_minimum_required(VERSION 3.10.1)
if(WIN32)
  file(GLOB source_files src/*.cpp)
else()
  file(GLOB source_files src/*.cpp include/*.hpp)
endif()

if(WIN32)
  set(system_shared_files SetupAPI Cfgmgr32)
else()
  set(system_shared_files "")
endif()

set(qt_shared_files Qt6::Core Qt6::Widgets Qt6::Gui)
set(pliugin_libfiles pluginmgr yjson)

set(plugin_name neousbplg)

add_library(${plugin_name} SHARED ${source_files})

set(resource_files icons/*.png)
load_resources(${plugin_name} ${resource_files})

target_link_libraries(${plugin_name} PRIVATE
  ${pliugin_libfiles}
  ${qt_shared_files}
  ${system_shared_files}
)
