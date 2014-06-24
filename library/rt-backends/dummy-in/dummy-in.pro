TEMPLATE = lib
TARGET = $$qtLibraryTarget(drumstick-rt-dummy-in)
DESTDIR = ../../../build/backends
include (../../../global.pri)
CONFIG += plugin create_prl #static
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
HEADERS += ../../include/rtmidiinput.h \
           ../../include/rtmidioutput.h \
           dummyinput.h

SOURCES += dummyinput.cpp

