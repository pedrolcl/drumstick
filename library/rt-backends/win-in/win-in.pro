TEMPLATE = lib
CONFIG += plugin
static {
    CONFIG += staticlib create_prl
}
TARGET = drumstick-rt-win-in
DESTDIR = ../../../build/lib/drumstick
DEPENDPATH += . ../../include
INCLUDEPATH += . ../../include
include (../../../global.pri)
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
QT -= gui

HEADERS += ../../include/drumstick/rtmidiinput.h \
           ../../include/drumstick/rtmidioutput.h \
           winmidiinput.h

SOURCES += \
           winmidiinput.cpp

LIBS += -lwinmm
