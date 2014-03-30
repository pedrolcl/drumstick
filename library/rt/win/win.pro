TEMPLATE = lib
CONFIG += plugin static
TARGET = drumstick-rt-win
DESTDIR = ../../../build/lib
OBJECTS_DIR = ../../../build/obj
MOC_DIR = ../../../build/moc
DEPENDPATH += . ../../include
INCLUDEPATH += . ../../include
include (../../../global.pri)
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

HEADERS += ../../include/rtmidiinput.h \
           ../../include/rtmidioutput.h \
           winmidioutput.h \
           winmidiinput.h

SOURCES += winmidioutput.cpp \
           winmidiinput.cpp

LIBS += -lwinmm
