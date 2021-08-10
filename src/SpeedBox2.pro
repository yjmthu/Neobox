DEFINES  -= UNICODE
DEFINES  += UMBCS
msvc {
    QMAKE_CFLAGS += /utf-8
    QMAKE_CXXFLAGS += /utf-8
}

VERSION = 21.8.10.0

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
CONFIG += release
#CONFIG += debug
DEFINES += QT_NO_DEBUG_OUTPUT

win32 {
    LIBS += -luser32
#    LIBS += -liphlpapi
#    LIBS += -lole32
#    LIBS += -luuid     #注册表需要
    LIBS += -loleaut32
#    LIBS += -ladvapi32 #注册表需要
#    LIBS += -lwininet
#    LIBS += -mwindows
}

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    YJson.cpp \
    YString.cpp \
    blankform.cpp \
    gmpoperatetip.cpp \
    main.cpp \
    funcbox.cpp \
    dialog.cpp \
    dialogwallpaper.cpp \
    form.cpp \
    menu.cpp \
    menuwallpaper.cpp \
    translater.cpp \
    tray.cpp \
    wallpaper.cpp \

HEADERS += \
    YJson.h \
    YString.h \
    blankform.h \
    dialog.h \
    dialogwallpaper.h \
    form.h \
    funcbox.h \
    gmpoperatetip.h \
    menu.h \
    menuwallpaper.h \
    translater.h \
    tray.h \
    wallpaper.h

FORMS += \
    dialog.ui \
    form.ui \
    translater.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# DISTFILES +=

RESOURCES += \
    res.qrc

RC_ICONS += icons/speedbox.ico
TARGET = SpeedBox
