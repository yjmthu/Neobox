# Generated from 3rd_qxtglobalshortcut.pri.

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui REQUIRED)

add_library(3rd_qxtglobalshortcut # WIN32 # MACOSX_BUNDLE
    qxtglobal.cpp qxtglobal.h
    qxtglobalshortcut.cpp qxtglobalshortcut.h qxtglobalshortcut_p.h
)

target_compile_definitions(3rd_qxtglobalshortcut PUBLIC
    BUILD_QXT_CORE
    BUILD_QXT_GUI
)

target_link_libraries(3rd_qxtglobalshortcut PUBLIC
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Core
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
        User32
    )
endif()
