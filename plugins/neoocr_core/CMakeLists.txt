cmake_minimum_required(VERSION 3.10.1)

if(WIN32)
  find_package(Leptonica CONFIG REQUIRED)
  find_package(Tesseract CONFIG REQUIRED)
  find_package(CURL CONFIG REQUIRED)
endif()

file(GLOB source_files src/*.cpp)

set(ocr_libfiles leptonica CURL::libcurl)

if(WIN32)
  list(APPEND ocr_libfiles Tesseract::libtesseract Iphlpapi)
else()
  list(APPEND ocr_libfiles tesseract)
endif()

add_library(neoocr_core STATIC ${source_files})

target_link_libraries(neoocr_core PUBLIC
  ${ocr_libfiles} pluginmgr yjson
)
