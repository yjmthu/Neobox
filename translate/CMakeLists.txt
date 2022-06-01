cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

include_directories(include)

add_library(translate STATIC
    include/text.h
    include/translater.h
    src/text.cpp
    src/translater.cpp
)

target_link_libraries(translate PRIVATE
    3rd_qxtglobalshortcut
    Qt::Widgets
    pthread
    lept
    tesseract
)
