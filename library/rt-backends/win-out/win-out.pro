TEMPLATE = lib
CONFIG += plugin create_prl #static
TARGET = $$qtLibraryTarget(drumstick-rt-win-out)
DESTDIR = ../../../build/backends
DEPENDPATH += . ../../include
INCLUDEPATH += . ../../include
include (../../../global.pri)
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
QT -= gui

HEADERS += ../../include/rtmidiinput.h \
           ../../include/rtmidioutput.h \
           winmidioutput.h

SOURCES += winmidioutput.cpp

LIBS += -lwinmm
