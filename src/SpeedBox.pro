# 使用多字节字符集
#DEFINES  -= UNICODE
#DEFINES  += UMBCS

# 使用msvc编译器和utf-8编码编译
msvc {
    QMAKE_CFLAGS += /utf-8
    QMAKE_CXXFLAGS += /utf-8
}

mingw {
}

#win32 {
#    LIBS += -luser32
#}


unix {
    QT += x11extras
    LIBS += -lX11
}

# 版本信息
VERSION = 22.0.0.0
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
QT       += network
QT += texttospeech
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
#greaterThan(5, QT_MAJOR_VERSION): QT += texttospeech

CONFIG += c++11

# release模式生成的文件更小，运行速度更快
# CONFIG += release
CONFIG += debug

# 禁用qDebug()
# DEFINES += QT_NO_DEBUG_OUTPUT

# 禁用 Qt 6之前的函数
# DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

include(3rd_qxtglobalshortcut/3rd_qxtglobalshortcut.pri)

SOURCES += \
    YJson.cpp \
    bingsetting.cpp \
    blankform.cpp \
    calculator.cpp \
    downloadprogress.cpp \
    globalfn.cpp \
    gmpoperatetip.cpp \
    main.cpp \
    funcbox.cpp \
    dialog.cpp \
    form.cpp \
    markdownnote.cpp \
    menu.cpp \
    netspeedhelper.cpp \
    roundclock.cpp \
    squareclock.cpp \
    systemfunctions.cpp \
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
    dialog.h \
    downloadprogress.h \
    form.h \
    funcbox.h \
    globalfn.h \
    gmpoperatetip.h \
    markdownnote.h \
    menu.h \
    netspeedhelper.h \
    qstylesheet.h \
    roundclock.h \
    speedwidget.h \
    squareclock.h \
    systemfunctions.h \
    translater.h \
    usbdrivehelper.h \
    wallpaper.h \
    windowposition.h


RESOURCES += \
    res.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# 生成的exe文件
TARGET = "SpeedBox"

FORMS += \
    ui/bingsetting.ui \
    ui/calculator.ui \
    ui/dialog.ui \
    ui/downloadprogress.ui \
    ui/translater.ui \
    ui/usbdrivehelper.ui
