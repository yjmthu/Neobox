include ($$(TRAINDEVHOME)/src/SppedBox.pri)

win32 {
    LIBS += -luser32
#    LIBS += -luuid
#    LIBS += -loleaut32
    LIBS += -ladvapi32
}

QMAKE_LFLAGS += /MANIFESTUAC:"level='requireAdministrator'"

TEMPLATE	= app
TARGET          = plreg
DESTDIR         = $$TRAIN_BIN_PATH
MOC_DIR         = temp/moc
RCC_DIR         = temp/rcc
UI_DIR          = temp/ui
OBJECTS_DIR     = temp/obj

SOURCES += \
        main.cpp

#RC_FILE += \
#        main.rc
