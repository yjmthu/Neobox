cmake_minimum_required(VERSION 3.10.1)

find_package(Qt6 REQUIRED COMPONENTS Svg)

if(WIN32)
  find_package(Leptonica CONFIG REQUIRED)
  find_package(Tesseract CONFIG REQUIRED)
endif()

file(GLOB source_files src/*.cpp)

# set(system_shared_files Shell32)
set(qt_shared_files Qt6::Core Qt6::Widgets Qt6::Gui Qt6::Svg)
set(pliugin_libfiles neoocr_core)
set(ocr_libfiles leptonica Tesseract::libtesseract)

add_library(neoocrplg SHARED ${source_files})

load_resources(neoocrplg icons/*)

target_link_libraries(neoocrplg PUBLIC
  ${pliugin_libfiles}
  ${qt_shared_files}
  ${ocr_libfiles}
  # ${system_shared_files}
)
