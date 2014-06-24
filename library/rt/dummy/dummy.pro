TEMPLATE = lib
TARGET = $$qtLibraryTarget(drumstick-rt-dummy)
DESTDIR = ../../../build/lib
include (../../../global.pri)
CONFIG += plugin static create_prl
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
HEADERS += ../../include/rtmidiinput.h \
           ../../include/rtmidioutput.h \
           dummyinput.h \
           dummyoutput.h

SOURCES += dummyinput.cpp \
           dummyoutput.cpp

