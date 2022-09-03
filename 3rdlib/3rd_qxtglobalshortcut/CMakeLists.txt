cmake_minimum_required(VERSION 3.10)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DBUILD_QXT_CORE -DBUILD_QXT_GUI)


if(WIN32)
    find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
else()
    find_package(Qt5 COMPONENTS Core Widgets X11Extras REQUIRED)
endif()

include_directories(include)

if(MACOS)
    set(EXTRE_SRC_FILES
        include/qxtwindowsystem_mac.h
        src/qxtglobalshortcut_mac.cpp
        src/qxtglobalshortcut_mac.cpp
    )
elseif(UNIX)
    set(EXTRE_SRC_FILES 
        include/x11info.h
        src/qxtglobalshortcut_x11.cpp
        src/qxtwindowsystem_x11.cpp
        src/x11info.cpp
    )
elseif(WIN32)
    set(EXTRE_SRC_FILES
        src/qxtwindowsystem_win.cpp
        src/qxtglobalshortcut_win.cpp
    )
endif()

add_library(3rd_qxtglobalshortcut
    include/qxtglobal.h
    include/qxtglobalshortcut.h
    src/qxtglobal.cpp
    src/qxtglobalshortcut.cpp
    ${EXTRE_SRC_FILES}
)

if(MACOS)

elseif(UNIX)
    target_link_libraries(3rd_qxtglobalshortcut PUBLIC
        Qt::Core
        Qt::Widgets
        Qt::X11Extras
        X11
        # cairo
    )
elseif(WIN32)
    target_link_libraries(3rd_qxtglobalshortcut PUBLIC
        User32
        Qt::Core
        Qt::Widgets
    )
endif()
