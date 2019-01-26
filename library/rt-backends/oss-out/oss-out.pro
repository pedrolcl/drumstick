TEMPLATE = lib
TARGET = drumstick-rt-oss-out
DESTDIR = ../../../build/lib/drumstick
include (../../../global.pri)
CONFIG += plugin
static {
    CONFIG += staticlib create_prl
}
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
QT -= gui

HEADERS += ../../include/drumstick/rtmidiinput.h \
           ../../include/drumstick/rtmidioutput.h \
           ossoutput.h

SOURCES += \
           ossoutput.cpp

