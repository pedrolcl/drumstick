TEMPLATE = lib
TARGET = drumstick-rt-synth
DESTDIR = ../../../build/lib
OBJECTS_DIR = ../../../build/obj
MOC_DIR = ../../../build/moc
include (../../../global.pri)
CONFIG += plugin static
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
HEADERS += ../../include/rtmidiinput.h \
           ../../include/rtmidioutput.h \
           synthengine.h \
           synthoutput.h

SOURCES += synthoutput.cpp synthengine.cpp

