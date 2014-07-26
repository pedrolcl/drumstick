TEMPLATE = lib
TARGET = $$qtLibraryTarget(drumstick-rt-oss-out)
DESTDIR = ../../../build/lib/drumstick
include (../../../global.pri)
CONFIG += plugin create_prl #static
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
QT -= gui

HEADERS += ../../include/rtmidiinput.h \
           ../../include/rtmidioutput.h \
           ossoutput.h

SOURCES += \
           ossoutput.cpp

