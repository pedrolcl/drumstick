TEMPLATE = lib
CONFIG += plugin create_prl #static
TARGET = $$qtLibraryTarget(drumstick-rt-net-in)
DESTDIR = ../../../build/backends
DEPENDPATH += . ../../include
INCLUDEPATH += . ../../include
include (../../../global.pri)
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
QT -= gui

HEADERS += ../../include/rtmidiinput.h \
           ../../include/rtmidioutput.h \
           netmidiinput.h \
           netmidiinput_p.h
SOURCES += netmidiinput.cpp \
           netmidiinput_p.cpp

QT += network
