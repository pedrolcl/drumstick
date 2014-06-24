TEMPLATE = lib
CONFIG += plugin static create_prl
TARGET = $$qtLibraryTarget(drumstick-rt-mac)
DESTDIR = ../../../build/lib
DEPENDPATH += . ../../include
INCLUDEPATH += . ../../include
include (../../../global.pri)
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

HEADERS += ../../include/rtmidiinput.h \
           ../../include/rtmidioutput.h \
           macmidiinput.h \
           macmidioutput.h \
           maccommon.h

SOURCES += macmidiinput.cpp \
           macmidioutput.cpp \
           maccommon.cpp

LIBS += -framework CoreMidi -framework CoreFoundation

#-framework CoreAudio -framework AudioToolbox -framework AudioUnit
