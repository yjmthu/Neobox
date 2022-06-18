cmake_minimum_required(VERSION 3.10)

project(widgets VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(Qt5 COMPONENTS Widgets Quick REQUIRED)
if (UNIX AND NOT MACOS)
  # Find KDE modules
  find_package(KF5WindowSystem REQUIRED)
endif()

include_directories(include)

add_executable(Neobox WIN32
    include/speedapp.h
    include/speedbox.h
    include/speedmenu.h
    include/pathdialog.h
    include/screenfetch.h
    src/main.cpp
    src/speedapp.cpp
    src/speedbox.cpp
    src/speedmenu.cpp
    src/pathdialog.cpp
    src/screenfetch.cpp
    resources.qrc
)

target_link_libraries(Neobox PRIVATE
    3rd_qxtglobalshortcut
    core
    wallpaper
    yjson
    Qt::Widgets
    Qt::Quick
    translate
)

if (UNIX)
  target_link_libraries(Neobox PRIVATE
        pthread
        # KF5::CoreAddons
        # KF5::I18n
        # KF5::WidgetsAddons
        KF5::WindowSystem
    )
endif()

