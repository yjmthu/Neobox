cmake_minimum_required(VERSION 3.10)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
qt_standard_project_setup()

find_package(Leptonica CONFIG REQUIRED)
find_package(Tesseract CONFIG REQUIRED)

if(WIN32)
elseif (UNIX AND NOT MACOS)
  # Find KDE modules
  find_package(KF5WindowSystem REQUIRED)
endif()

include_directories(include)

set(SRC_LIST
    src/main.cpp
    src/varbox.cpp
    src/speedbox.cpp
    src/neomenu.cpp
    src/screenfetch.cpp
    src/qimage2text.cpp
    src/translate.cpp
    resources.qrc
)

set(CMAKE_AUTORCC ON)

qt_add_executable(Neobox WIN32
  ${SRC_LIST}
)

target_link_libraries(Neobox PRIVATE
    core
    wallpaper
    yjson
    leptonica
    Tesseract::libtesseract
    Qt6::Core
    Qt6::Widgets
)

if (UNIX)
  target_link_libraries(Neobox PRIVATE
        pthread
        KF5::WindowSystem
    )
endif()

