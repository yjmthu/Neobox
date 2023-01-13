cmake_minimum_required(VERSION 3.10.1)

file(GLOB source_files src/*.cpp include/*.hpp)

file(GLOB _resource_files icons/*.png)
set(resource_files "")
foreach(full_filepath ${_resource_files})
  file(RELATIVE_PATH file_path ${CMAKE_CURRENT_SOURCE_DIR} ${full_filepath})
  message(STATUS "res='${file_path}'")
  list(APPEND resource_files ${file_path})
endforeach()

set(system_shared_files Shell32)
set(qt_shared_files Qt6::Core Qt6::Widgets Qt6::Gui)
set(pliugin_libfiles translate_core yjson pluginmgr)

add_library(neotranslateplg SHARED
  ${source_files}
)

target_link_libraries(neotranslateplg PUBLIC
  ${pliugin_libfiles}
  ${qt_shared_files}
  ${system_shared_files}
)

qt_add_resources(pluginmgr "neotranslateplg"
  PREFIX "/"
  FILES ${resource_files}
)
