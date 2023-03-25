cmake_minimum_required(VERSION 3.10.1)

find_package(Leptonica CONFIG REQUIRED)
find_package(Tesseract CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)

file(GLOB source_files src/*.cpp)

set(qt_shared_files Qt6::Core Qt6::Widgets Qt6::Gui Qt6::Svg)

if(WIN32)
  set(ocr_libfiles)
else()
  set(ocr_libfiles leptonica tesseract)
endif()

add_library(neoocrplg SHARED ${source_files})

load_resources(neoocrplg icons/*)

target_link_libraries(neoocrplg PRIVATE
  ${qt_shared_files}
  ${ocr_libfiles}
  pluginmgr yjson
  leptonica
  Tesseract::libtesseract
  CURL::libcurl
)
