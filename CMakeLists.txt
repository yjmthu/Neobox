cmake_minimum_required(VERSION 3.5)

project(Neobox VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(CMAKE_BUILD_TYPE "Debug")
# SET(CMAKE_BUILD_TYPE "Release")

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check https://doc.qt.io/qt/deployment-android.html for more information.
# They need to be set before the find_package( ...) calls below.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
set_property(TARGET Qt5::Core PROPERTY INTERFACE_COMPILE_FEATURES "")
add_compile_options("-std=c++1z")

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

        wallpaper/apiclass.cpp
        wallpaper/apiclass.hpp
        wallpaper/wallpaper.cpp
        wallpaper/wallpaper.h

        core/appcode.hpp
        core/netspeedhelper.cpp
        core/netspeedhelper.h
)

qt5_add_resources(QRC_FILES ${QRC_SOURCE_FILES})
SOURCE_GROUP("Resource Files" FILES ${QRC_SOURCE_FILES})

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Neobox
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${QRC_FILES}
    )
else()
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
endif()

if (WIN32)
    target_link_libraries(Neobox PRIVATE
        3rd_qxtglobalshortcut
        yjson
        Qt${QT_VERSION_MAJOR}::Widgets
        libssl
        libcrypto
    )
elseif (UNIX)
    target_link_libraries(Neobox PRIVATE
        3rd_qxtglobalshortcut
        yjson
        Qt${QT_VERSION_MAJOR}::Widgets
        pthread
        ssl
        crypto
    )
endif()

set_target_properties(Neobox PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

target_compile_features(Neobox PRIVATE cxx_std_17)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Neobox)
endif()

