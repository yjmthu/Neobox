cmake_minimum_required(VERSION 3.10.1)

find_package(Qt6 REQUIRED COMPONENTS Svg)
find_package(Leptonica CONFIG REQUIRED)
file(GLOB source_files src/*.cpp)

set(system_shared_files Shell32)
set(qt_shared_files Qt6::Core Qt6::Widgets Qt6::Gui Qt6::Svg)
set(pliugin_libfiles neoocr_core pluginmgr yjson)
set(ocr_libfiles leptonica)

add_library(neoocrplg SHARED ${source_files} 
)

file(GLOB _resource_files icons/*)
set(resource_files "")
foreach(full_filepath ${_resource_files})
  file(RELATIVE_PATH file_path ${CMAKE_CURRENT_SOURCE_DIR} ${full_filepath})
  message(STATUS "res='${file_path}'")
  list(APPEND resource_files ${file_path})
endforeach()

qt_add_resources(neoocrplg "neoocrplg"
  PREFIX "/"
  FILES ${resource_files}
)

target_link_libraries(neoocrplg PRIVATE
  ${pliugin_libfiles}
  ${qt_shared_files}
  ${ocr_libfiles}
  ${system_shared_files}
)
