cmake_minimum_required(VERSION 3.10.1)

file(GLOB source_files src/*.cpp)

set(system_shared_files Shell32 PowrProf)
set(qt_shared_files Qt6::Core Qt6::Widgets Qt6::Gui)
set(pliugin_libfiles pluginmgr yjson)

add_library(neosystemplg SHARED ${source_files})

file(GLOB _resource_files icons/*)

set(resource_files "")
foreach(full_filepath ${_resource_files})
  file(RELATIVE_PATH file_path ${CMAKE_CURRENT_SOURCE_DIR} ${full_filepath})
  message(STATUS "res='${file_path}'")
  list(APPEND resource_files ${file_path})
endforeach()

qt_add_resources(neosystemplg "neosystemplg"
    PREFIX "/"
    FILES ${resource_files}
)

target_link_libraries(neosystemplg PRIVATE
  ${pliugin_libfiles}
  ${qt_shared_files}
  ${system_shared_files}
)


