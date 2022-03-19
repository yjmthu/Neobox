# Generated from 3rd_qxtglobalshortcut.pri.

cmake_minimum_required(VERSION 3.16)
project(3rd_qxtglobalshortcut VERSION 1.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}")

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui)
find_package(Qt${QT_VERSION_MAJOR} OPTIONAL_COMPONENTS X11Extras)

qt_add_executable(3rd_qxtglobalshortcut WIN32 MACOSX_BUNDLE
    qxtglobal.cpp qxtglobal.h
    qxtglobalshortcut.cpp qxtglobalshortcut.h qxtglobalshortcut_p.h
)
target_compile_definitions(3rd_qxtglobalshortcut PUBLIC
    BUILD_QXT_CORE
    BUILD_QXT_GUI
)

target_link_libraries(3rd_qxtglobalshortcut PUBLIC
    Qt::Core
    Qt::Gui
)

if(UNIX AND NOT MACOS)
    target_sources(3rd_qxtglobalshortcut PUBLIC
        qxtglobalshortcut_x11.cpp
        qxtwindowsystem_x11.cpp
        x11info.h
    )

    target_link_libraries(3rd_qxtglobalshortcut PUBLIC
        Qt::X11Extras
    )
endif()

if(MACOS)
    target_sources(3rd_qxtglobalshortcut PUBLIC
        qxtglobalshortcut_mac.cpp
        qxtwindowsystem_mac.cpp qxtwindowsystem_mac.h
    )
endif()

if(WIN32)
    target_sources(3rd_qxtglobalshortcut PUBLIC
        qxtglobalshortcut_win.cpp
        qxtwindowsystem_win.cpp
    )

    target_link_libraries(3rd_qxtglobalshortcut PUBLIC
        user32
    )
endif()

install(TARGETS 3rd_qxtglobalshortcut
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
