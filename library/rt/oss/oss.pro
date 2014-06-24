TEMPLATE = lib
TARGET = $$qtLibraryTarget(drumstick-rt-oss)
DESTDIR = ../../../build/lib
include (../../../global.pri)
CONFIG += plugin static create_prl
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
HEADERS += ../../include/rtmidiinput.h \
           ../../include/rtmidioutput.h \
           ossinput_p.h \
           ossinput.h \
           ossoutput.h

SOURCES += ossinput.cpp \
           ossinput_p.cpp \
           ossoutput.cpp

