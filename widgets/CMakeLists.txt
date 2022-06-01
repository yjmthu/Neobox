cmake_minimum_required(VERSION 3.10)

project(widgets VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options("/utf-8")
    add_compile_options("/std:c++latest")
endif ()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(Qt5 COMPONENTS Widgets REQUIRED)

if(WIN32)
    include_directories(
        C:/MSVC2022/leptonica-1.82.0/include
        C:/MSVC2022/OpenSSL/OpenSSL-Win64/include
        C:/MSVC2022/tesseract/include
    )
endif()

include_directories(include)

# qt5_add_resources(COLYOO_QT_RESOURCES resources.qrc)

add_executable(widgets
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
    core
    wallpaper
    yjson
    Qt5::Widgets
    lept
    tesseract
    translate
)

if (WIN32)
    target_link_libraries(widgets PRIVATE
        ${M_STATIC_LIBS}
        libssl
        libcrypto
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
