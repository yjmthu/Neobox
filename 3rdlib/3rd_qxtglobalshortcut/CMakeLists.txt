cmake_minimum_required(VERSION 3.10)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DQT_NO_DEBUG_OUTPUT -DNO_WARN_X86_INTRINSICS)

include_directories(include)

if(UNIX)
    find_package(Qt5 COMPONENTS Core Widgets X11Extras REQUIRED)
endif()

add_library(3rd_qxtglobalshortcut
    include/qxtglobal.h
    include/qxtglobalshortcut.h
    include/qxtglobalshortcut_p.h
    src/qxtglobal.cpp
    src/qxtglobalshortcut.cpp
    include/x11info.h
    src/qxtglobalshortcut_x11.cpp
    src/qxtwindowsystem_x11.cpp
    src/x11info.cpp
)

target_compile_definitions(3rd_qxtglobalshortcut PUBLIC
    BUILD_QXT_CORE
    BUILD_QXT_GUI
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
    target_sources(3rd_qxtglobalshortcut PUBLIC
        src/qxtglobalshortcut_win.cpp
        src/qxtwindowsystem_win.cpp
    )

    target_link_libraries(3rd_qxtglobalshortcut PUBLIC
        User32
    )
endif()
