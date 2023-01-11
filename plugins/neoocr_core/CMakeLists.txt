cmake_minimum_required(VERSION 3.10.1)

find_package(Leptonica CONFIG REQUIRED)
find_package(Tesseract CONFIG REQUIRED)

file(GLOB source_files src/*.cpp)

add_library(neoocr_core STATIC ${source_files})

set(ocr_libfiles leptonica Tesseract::libtesseract)

target_link_libraries(neoocr_core PUBLIC
  ${ocr_libfiles}
  pluginmgr yjson
)
