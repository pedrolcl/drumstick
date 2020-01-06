TEMPLATE = lib
CONFIG += c++11 plugin
static {
    CONFIG += staticlib create_prl
}
TARGET = drumstick-rt-alsa-in
DESTDIR = ../../../build/lib/drumstick
DEPENDPATH += . ../../include
INCLUDEPATH += . ../../include
include (../../../global.pri)
QT -= gui

HEADERS += alsamidiinput.h
SOURCES += alsamidiinput.cpp

LIBS += -L../../../build/lib \
        -ldrumstick-rt \
        -ldrumstick-alsa \
        -lasound
