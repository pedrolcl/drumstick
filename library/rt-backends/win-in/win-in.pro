TEMPLATE = lib
CONFIG += plugin
static {
    CONFIG += staticlib create_prl
}
TARGET = $$qtLibraryTarget(drumstick-rt-win-in)
DESTDIR = ../../../build/lib/drumstick
DEPENDPATH += . ../../include
INCLUDEPATH += . ../../include
include (../../../global.pri)
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
QT -= gui

HEADERS += ../../include/rtmidiinput.h \
           ../../include/rtmidioutput.h \
           winmidiinput.h

SOURCES += \
           winmidiinput.cpp

LIBS += -lwinmm

win32 {
    TARGET_EXT = .dll
}
