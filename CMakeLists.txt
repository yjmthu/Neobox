cmake_minimum_required(VERSION 3.5)

project(Neobox VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
add_compile_options(-std=c++20)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_BUILD_TYPE "Release")

find_package(QT NAMES Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
# set_property(TARGET Qt5::Core PROPERTY INTERFACE_COMPILE_FEATURES "")
# add_compile_options("-std=c++1z")

set(YJSON_DIR ${CMAKE_SOURCE_DIR}/../YJson)

include_directories(
    ${YJSON_DIR}
)

add_subdirectory(${YJSON_DIR} yjson)
add_subdirectory(3rdlib/3rd_qxtglobalshortcut 3rd_qxtglobalshortcut)

set(QRC_SOURCE_FILES recourses.qrc)

set(PROJECT_SOURCES
        main.cpp

        widgets/speedapp.cpp
        widgets/speedapp.h
        widgets/speedbox.cpp
        widgets/speedbox.h
        widgets/speedmenu.cpp
        widgets/speedmenu.h
        widgets/pathdialog.cpp
        widgets/pathdialog.h
        widgets/screenfetch.cpp
        widgets/screenfetch.h

        wallpaper/apiclass.cpp
        wallpaper/apiclass.hpp
        wallpaper/wallpaper.cpp
        wallpaper/wallpaper.h

        core/appcode.hpp
        core/netspeedhelper.cpp
        core/netspeedhelper.h

        translate/translater.cpp
        translate/translater.h
        translate/text.cpp
        translate/text.h
)

qt5_add_resources(QRC_FILES ${QRC_SOURCE_FILES})
SOURCE_GROUP("Resource Files" FILES ${QRC_SOURCE_FILES})

if(ANDROID)
    add_library(Neobox SHARED
        ${PROJECT_SOURCES}
        ${QRC_FILES}
    )
else()
    add_executable(Neobox
        ${PROJECT_SOURCES}
        ${QRC_FILES}
    )
endif()

if (WIN32)
    target_link_libraries(Neobox PRIVATE
        3rd_qxtglobalshortcut
        yjson
        Qt${QT_VERSION_MAJOR}::Widgets
        libssl
        libcrypto
        lept
        tesseract
    )
elseif (UNIX)
    target_link_libraries(Neobox PRIVATE
        3rd_qxtglobalshortcut
        yjson
        Qt${QT_VERSION_MAJOR}::Widgets
        pthread
        ssl
        crypto
        lept
        tesseract
    )
endif()

set_target_properties(Neobox PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

target_compile_features(Neobox PRIVATE cxx_std_20)
