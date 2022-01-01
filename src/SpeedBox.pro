# 使用多字节字符集
#DEFINES  -= UNICODE
#DEFINES  += UMBCS

# 使用msvc编译器和utf-8编码编译
msvc {
    QMAKE_CFLAGS += /utf-8
    QMAKE_CXXFLAGS += /utf-8
}

# 版本信息
VERSION = 21.12.3.0
# 图标
RC_ICONS += icons/speedbox.ico
# 公司名称
# QMAKE_TARGET_COMPANY = "NULL"
# 产品名称
QMAKE_TARGET_PRODUCT = "Speed Box"
# 文件说明
QMAKE_TARGET_DESCRIPTION = "Speed Box"
# 版权信息
QMAKE_TARGET_COPYRIGHT = "Copyright (C) 2021"
# 中文（中国）
RC_LANG = 0x0804


QT       += core gui
QT += network
QT += core5compat
# QT += texttospeech
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# release模式生成的文件更小，运行速度更快
#CONFIG += release
CONFIG += debug

# 禁用qDebug()
# DEFINES += QT_NO_DEBUG_OUTPUT

# 禁用 Qt 6之前的函数
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

SOURCES += \
    YJson.cpp \
    bingsetting.cpp \
    blankform.cpp \
    calculator.cpp \
    desktopmask.cpp \
    downloadprogress.cpp \
    formsetting.cpp \
    gmpoperatetip.cpp \
    main.cpp \
    funcbox.cpp \
    dialog.cpp \
    form.cpp \
    menu.cpp \
    netspeedhelper.cpp \
    translater.cpp \
    usbdrivehelper.cpp \
    wallpaper.cpp \

HEADERS += \
    FormulaPaser.h \
    YEncode.h \
    YJson.h \
    YString.h \
    bingsetting.h \
    blankform.h \
    calculator.h \
    desktopmask.h \
    dialog.h \
    downloadprogress.h \
    form.h \
    formsetting.h \
    funcbox.h \
    gmpoperatetip.h \
    menu.h \
    netspeedhelper.h \
    speedwidget.h \
    translater.h \
    usbdrivehelper.h \
    wallpaper.h

FORMS += \
    bingsetting.ui \
    calculator.ui \
    dialog.ui \
    downloadprogress.ui \
    form.ui \
    formsetting.ui \
    translater.ui \
    usbdrivehelper.ui


RESOURCES += \
    res.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# 生成的exe文件
TARGET = "SpeedBox"
