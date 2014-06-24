TEMPLATE = lib
TARGET = $$qtLibraryTarget(drumstick-rt-synth)
DESTDIR = ../../../build/lib
include (../../../global.pri)
CONFIG += plugin create_prl #static
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
HEADERS += ../../include/rtmidiinput.h \
           ../../include/rtmidioutput.h \
           synthengine.h \
           synthoutput.h

SOURCES += synthoutput.cpp synthengine.cpp

