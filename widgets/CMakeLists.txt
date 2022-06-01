cmake_minimum_required(VERSION 3.10)

project(widgets VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(Qt5 COMPONENTS Widgets REQUIRED)

include_directories(include)

add_executable(widgets WIN32
    include/mainloop.h
    include/speedapp.h
    include/speedbox.h
    include/speedmenu.h
    include/pathdialog.h
    include/screenfetch.h
    src/mainloop.cpp
    src/speedapp.cpp
    src/speedbox.cpp
    src/speedmenu.cpp
    src/pathdialog.cpp
    src/screenfetch.cpp
    resources.qrc
)

set(M_STATIC_LIBS
    3rd_qxtglobalshortcut
    core
    wallpaper
    yjson
    Qt5::Widgets
    translate
)

if (WIN32)
    target_link_libraries(widgets PRIVATE
        ${M_STATIC_LIBS}
        libssl_static
        libcrypto_static
    )
elseif (UNIX)
    target_link_libraries(widgets PRIVATE
        ${M_STATIC_LIBS}
        pthread
        ssl
        crypto
    )
endif()

# target_compile_features(widgets PRIVATE cxx_std_20)
