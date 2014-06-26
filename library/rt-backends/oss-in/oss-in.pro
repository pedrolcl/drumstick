TEMPLATE = lib
TARGET = $$qtLibraryTarget(drumstick-rt-oss-in)
DESTDIR = ../../../build/backends
include (../../../global.pri)
CONFIG += plugin create_prl #static
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
QT -= gui

HEADERS += ../../include/rtmidiinput.h \
           ../../include/rtmidioutput.h \
           ossinput_p.h \
           ossinput.h

SOURCES += ossinput.cpp \
           ossinput_p.cpp

