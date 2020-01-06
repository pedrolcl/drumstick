TEMPLATE = lib
CONFIG += plugin
static {
    CONFIG += staticlib create_prl
}
TARGET = drumstick-rt-alsa-out
DESTDIR = ../../../build/lib/drumstick
DEPENDPATH += . ../../include
INCLUDEPATH += . ../../include
include (../../../global.pri)
QT -= gui

HEADERS += alsamidioutput.h
SOURCES += alsamidioutput.cpp

LIBS += -L../../../build/lib \
        -ldrumstick-rt \
        -ldrumstick-alsa \
        -lasound
