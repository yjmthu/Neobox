cmake_minimum_required(VERSION 3.10.1)

if(WIN32)
  if (NOT TARGET Leptonica)
    find_package (Leptonica CONFIG)
    if (NOT Leptonica_FOUND)
      message (WARNING \"Leptonica not found, skipping this CMakeLists.txt file.\")
      return()
    endif()
  endif()

  if (NOT TARGET Tesseract)
    find_package (Tesseract CONFIG)
    find_package(CURL CONFIG)
    if (NOT Tesseract_FOUND)
      message (WARNING \"Tesseract not found, skipping this CMakeLists.txt file.\")
      return()
    endif()
  endif()

  if (NOT TARGET CURL)
    find_package(CURL CONFIG)
    if (NOT CURL_FOUND)
      message (WARNING \"CURL not found, skipping this CMakeLists.txt file.\")
      return()
    endif()
  endif()
endif()

file(GLOB source_files src/*.cpp)

set(qt_shared_files Qt6::Core Qt6::Widgets Qt6::Gui)

if(WIN32)
  set(ocr_libfiles Tesseract::libtesseract CURL::libcurl leptonica)
else()
  set(ocr_libfiles leptonica tesseract)
endif()

set(plugin_name neoocrplg)
add_library(${plugin_name} SHARED ${source_files})
target_include_directories(${plugin_name} PRIVATE include)

set(resource_files icons/*.png)
load_resources(${plugin_name} resource_files)

target_link_libraries(${plugin_name} PRIVATE
  ${qt_shared_files}
  ${ocr_libfiles}
  pluginmgr
)

install_plugin(${plugin_name})
