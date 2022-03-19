cmake_minimum_required(VERSION 3.5)

set(CMAKE_C_COMPILER "C:/MinGW/mingw64/bin/gcc.exe")
set(CMAKE_CXX_COMPILER "C:/MinGW/mingw64/bin/g++.exe")

if (WIN32)
    # add_compile_options("/utf-8")
    add_definitions(-DUNICODE -D_UNICODE)
endif ()

project(SPEEDBOX  VERSION 0.1 LANGUAGES CXX)

# set(CMAKE_AUTOUIC ON)
# set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(SPEEDBOX_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
# set(SPEEDBOX_SOURCE_DIR ./)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# 这些生成的中间文件都会在build目录下，这样的话，编译器则不能定位由uic程序产生的诸如_ui_mainwindow.h等文件。所以，我们需要把build目录添加到包含目录中
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    "${PROJECT_SOURCE_DIR}/3rd_qxtglobalshortcut"
)

add_subdirectory(3rd_qxtglobalshortcut)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets Network TextToSpeech REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets Network TextToSpeech REQUIRED)

add_definitions(-DQT_NO_DEBUG_OUTPUT)
# set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_BUILD_TYPE RELEASE)

# 添加所有的头文件
SET(SPEEDBOX_HEADERS 
    aboutauthor.h
    formulapaser.h
    yencode.h
    yjson.h
    ystring.h
    aboutnew.h
    bingsetting.h
    blankform.h
    calculator.h
    dialog.h
    form.h
    funcbox.h
    globalfn.h
    gmpoperatetip.h
    markdownnote.h
    menu.h
    netspeedhelper.h
    qstylesheet.h
    roundclock.h
    speedwidget.h
    squareclock.h
    systemfunctions.h
    translater.h
    usbdrivehelper.h
    wallpaper.h
    windowposition.h
)

set(SPEEDBOX_SOURCES
    main.cpp
    aboutauthor.cpp
    yjson.cpp
    aboutnew.cpp
    bingsetting.cpp
    blankform.cpp
    calculator.cpp
    globalfn.cpp
    gmpoperatetip.cpp
    funcbox.cpp
    dialog.cpp
    form.cpp
    markdownnote.cpp
    menu.cpp
    netspeedhelper.cpp
    roundclock.cpp
    squareclock.cpp
    systemfunctions.cpp
    translater.cpp
    usbdrivehelper.cpp
    wallpaper.cpp
)

# 添加所有的.ui文件
SET(SPEEDBOX_FORMS 
    ui/aboutauthor.ui
    ui/bingsetting.ui
    ui/calculator.ui
    ui/dialog.ui
    ui/downloadprogress.ui
    ui/translater.ui
    ui/usbdrivehelper.ui
)

# 添加资源文件
SET(SPEEDBOX_RESOURCES 
    res.qrc
)

# 调用预编译器moc，需要使用 QT5_WRAP_CPP宏
QT5_WRAP_CPP(SPEEDBOX_HEADERS_MOC ${SPEEDBOX_HEADERS})
# 使用uic处理.ui文件
QT5_WRAP_UI(SPEEDBOX_FORMS_HEADERS ${SPEEDBOX_FORMS})
# 使用rcc处理.qrc文件
QT5_ADD_RESOURCES(SPEEDBOX_RESOURCES_RCC ${SPEEDBOX_RESOURCES})

#生成可执行文件
add_executable(SpeedBox
    ${SPEEDBOX_SOURCES}
    ${SPEEDBOX_HEADERS_MOC}
    ${SPEEDBOX_FORMS_HEADERS}
    ${SPEEDBOX_RESOURCES_RCC}
)

# 这句放在这似乎不管用
# target_compile_definitions(SpeedBox PUBLIC QT_DEBUG_NO_OUTPUT)

#为target添加需要链接的共享库
target_link_libraries(SpeedBox
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::TextToSpeech
    User32
)

target_link_libraries(SpeedBox 3rd_qxtglobalshortcut)
