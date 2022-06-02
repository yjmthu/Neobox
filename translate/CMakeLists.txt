cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

if(WIN32)
    if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        set(tesseract
            zlibstatic
            libpng16_static
            tiff
            leptonica-1.82.0
            tesseract51d
        )
    elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
        set(tesseract
            zlibstatic
            libpng16_static
            tiff
            leptonica-1.82.0
            tesseract51
        )
    endif()
    include_directories(${LEPTONICA_INCLUDE_DIRS})
elseif(UNIX)
    set(tesseract
        tesseract
        lept
        pthread
    )
endif()

include_directories(include)

add_library(translate STATIC
    include/text.h
    include/translater.h
    src/text.cpp
    src/translater.cpp
)

target_link_libraries(translate PRIVATE
    Qt::Core
    3rd_qxtglobalshortcut
    httplib
    ${tesseract}
)
