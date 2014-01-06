TEMPLATE = lib
TARGET = drumstick-rt-dummy
DESTDIR = ../../../build/lib
OBJECTS_DIR = ../../../build/obj
MOC_DIR = ../../../build/moc
include (../../../global.pri)
CONFIG += plugin static
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
HEADERS += ../../include/rtmidiinput.h \
           ../../include/rtmidioutput.h \
           dummyinput.h \
           dummyoutput.h

SOURCES += dummyinput.cpp \
           dummyoutput.cpp

