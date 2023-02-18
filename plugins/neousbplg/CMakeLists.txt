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

file(GLOB _resource_files icons/*.png)

set(resource_files "")
foreach(full_filepath ${_resource_files})
  file(RELATIVE_PATH file_path ${CMAKE_CURRENT_SOURCE_DIR} ${full_filepath})
  message(STATUS "res='${file_path}'")
  list(APPEND resource_files ${file_path})
endforeach()

qt_add_resources(${plugin_name} "neousbplg"
  PREFIX "/"
  FILES ${resource_files}
)

target_link_libraries(${plugin_name} PRIVATE
  ${pliugin_libfiles}
  ${qt_shared_files}
  ${system_shared_files}
)
