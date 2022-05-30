# Generated from 3rd_qxtglobalshortcut.pri.
cmake_minimum_required(VERSION 3.5)

# set(CMAKE_C_COMPILER "C:/MinGW/mingw64/bin/gcc.exe")
# set(CMAKE_CXX_COMPILER "C:/MinGW/mingw64/bin/g++.exe")

project(3rd_qxtglobalshortcut  VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 这些生成的中间文件都会在build目录下，这样的话，编译器则不能定位由uic程序产生的诸如_ui_mainwindow.h等文件。所以，我们需要把build目录添加到包含目录中

add_definitions(-DQT_NO_DEBUG_OUTPUT -DNO_WARN_X86_INTRINSICS)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)
if(WIN32)
    find_package(QT NAMES Qt5 COMPONENTS Core Gui Widgets REQUIRED)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets REQUIRED)
elseif(UNIX)
    find_package(QT NAMES Qt5 COMPONENTS Core Gui Widgets X11Extras REQUIRED)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets X11Extras REQUIRED)
endif()
# set(CMAKE_BUILD_TYPE RELEASE)

add_library(3rd_qxtglobalshortcut
    ${CMAKE_CURRENT_SOURCE_DIR}/qxtglobal.cpp ${CMAKE_CURRENT_SOURCE_DIR}/qxtglobal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qxtglobalshortcut.cpp ${CMAKE_CURRENT_SOURCE_DIR}/qxtglobalshortcut.h ${CMAKE_CURRENT_SOURCE_DIR}/qxtglobalshortcut_p.h
)

target_compile_definitions(3rd_qxtglobalshortcut PUBLIC
    BUILD_QXT_CORE
    BUILD_QXT_GUI
)

target_link_libraries(3rd_qxtglobalshortcut PUBLIC
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
)

if(UNIX AND NOT MACOS)
    target_sources(3rd_qxtglobalshortcut PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/qxtglobalshortcut_x11.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qxtwindowsystem_x11.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/x11info.h
    )

    target_link_libraries(3rd_qxtglobalshortcut PUBLIC
        Qt::X11Extras
        X11
        # cairo
    )
endif()

if(MACOS)
    target_sources(3rd_qxtglobalshortcut PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/qxtglobalshortcut_mac.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qxtwindowsystem_mac.cpp qxtwindowsystem_mac.h
    )
endif()

if(WIN32)
    target_sources(3rd_qxtglobalshortcut PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/qxtglobalshortcut_win.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qxtwindowsystem_win.cpp
    )

    target_link_libraries(3rd_qxtglobalshortcut PUBLIC
        User32
    )
endif()
