cmake_minimum_required(VERSION 3.10.1)

if(WIN32)
  find_package(Leptonica CONFIG REQUIRED)
  find_package(Tesseract CONFIG REQUIRED)
endif()

file(GLOB source_files src/*.cpp)

add_library(neoocr_core STATIC ${source_files})

if(WIN32)
  set(ocr_libfiles leptonica Tesseract::libtesseract)
else()
  set(ocr_libfiles leptonica tesseract)
endif()
target_link_libraries(neoocr_core PUBLIC
  ${ocr_libfiles}
  pluginmgr yjson
)
