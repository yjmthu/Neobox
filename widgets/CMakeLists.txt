cmake_minimum_required(VERSION 3.10)

project(widgets VERSION 0.1 LANGUAGES CXX)
# set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets Quick REQUIRED)
find_package(Leptonica CONFIG REQUIRED)
find_package(Tesseract CONFIG REQUIRED)

if(WIN32)
elseif (UNIX AND NOT MACOS)
  # Find KDE modules
  find_package(KF5WindowSystem REQUIRED)
endif()

include_directories(include)

set(SRC_LIST
    include/varbox.h
    include/speedbox.h
    include/speedmenu.h
    include/pixmapimage.h
    src/main.cpp
    src/varbox.cpp
    src/speedbox.cpp
    src/speedmenu.cpp
    src/pixmapimage.cpp
    resources.qrc
)

if (WIN32)
    add_executable(Neobox WIN32
      ${SRC_LIST}
    )
elseif (UNIX)
    add_executable(Neobox
      ${SRC_LIST}
    )
endif()

target_link_libraries(Neobox PRIVATE
    3rd_qxtglobalshortcut
    core
    wallpaper
    yjson
    Qt::Widgets
    Qt::Quick
    leptonica
    Tesseract::libtesseract
)

if (UNIX)
  target_link_libraries(Neobox PRIVATE
        pthread
        KF5::WindowSystem
    )
endif()

