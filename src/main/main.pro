include ($$(TRAINDEVHOME)/src/SppedBox.pri)
include($$(TRAINDEVHOME)/src/qxtglobalshortcut5/qxt.pri)

# QT += network

win32 {
    LIBS += -luser32
#    LIBS += -liphlpapi
#    LIBS += -lole32
    LIBS += -luuid
    LIBS += -loleaut32
    LIBS += -ladvapi32
    LIBS += -lwininet
#    LIBS += -mwindows
}

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE    = app
TARGET      = speedbox
DESTDIR     = $$DEVHOME
MOC_DIR     = temp/moc
RCC_DIR     = temp/rcc
UI_DIR      = temp/ui
OBJECTS_DIR = temp/obj

RC_ICONS += icons/exe.ico

SOURCES += \
    main.cpp \
    funcbox.cpp \
    dialog.cpp \
    dialogwallpaper.cpp \
    form.cpp \
    menu.cpp \
    menuwallpaper.cpp \
    translater.cpp \
    wallpaper.cpp \
    $$TRAIN_SRC_PATH/md5/md5.cpp \
    $$TRAIN_SRC_PATH/bueatray/bueatray.cpp

HEADERS += \
    $$TRAIN_INCLUDE_PATH/dialog.h \
    $$TRAIN_INCLUDE_PATH/dialogwallpaper.h \
    $$TRAIN_INCLUDE_PATH/form.h \
    $$TRAIN_INCLUDE_PATH/funcbox.h \
    $$TRAIN_INCLUDE_PATH/md5.h \
    $$TRAIN_INCLUDE_PATH/menu.h \
    $$TRAIN_INCLUDE_PATH/menuwallpaper.h \
    $$TRAIN_INCLUDE_PATH/translater.h \
    $$TRAIN_INCLUDE_PATH/wallpaper.h \
    $$TRAIN_INCLUDE_PATH/bueatray.h

FORMS += \
    dialog.ui \
    form.ui \
    translater.ui

RESOURCES += \
    res.qrc
