cmake_minimum_required(VERSION 3.10.1)

find_package(Leptonica CONFIG REQUIRED)
find_package(Tesseract CONFIG REQUIRED)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Svg UiTools)
qt_standard_project_setup()

file(GLOB source_files src/*.c src/*.cpp)
file(GLOB _resource_files fonts/*.ttf  icons/*  jsons/*.json scripts/* styles/*)

set(resource_files "")
foreach(full_filepath ${_resource_files})
  file(RELATIVE_PATH file_path ${CMAKE_CURRENT_SOURCE_DIR} ${full_filepath})
  message(STATUS "res='${file_path}'")
  list(APPEND resource_files ${file_path})
endforeach()

add_library(neobox_core STATIC ${source_files})

qt_add_resources(neobox_core "neobox"
  PREFIX "/"
  FILES ${resource_files}
)

set(qt_shared_files Qt6::Core Qt6::Widgets Qt6::Gui Qt6::Svg Qt6::UiTools)
set(ocr_libfiles leptonica Tesseract::libtesseract)
set(project_libfiles network pluginmgr core yjson)

target_link_libraries(neobox_core PRIVATE
  ${ocr_libfiles}
  ${project_libfiles}
  ${qt_shared_files}
)
