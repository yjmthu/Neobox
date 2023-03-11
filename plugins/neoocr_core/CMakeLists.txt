cmake_minimum_required(VERSION 3.10.1)

if(WIN32)
  find_package(Leptonica CONFIG REQUIRED)
  find_package(Tesseract CONFIG REQUIRED)
endif()

file(GLOB source_files src/*.cpp)
add_library(neoocr_core STATIC ${source_files})

set(ocr_libfiles leptonica)

if(WIN32)
  list(APPEND ocr_libfiles Tesseract::libtesseract)
else()
  list(APPEND ocr_libfiles tesseract)
endif()

target_link_libraries(neoocr_core PUBLIC
  ${ocr_libfiles} pluginmgr yjson
)
