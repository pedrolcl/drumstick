TEMPLATE = lib
CONFIG += plugin #create_prl #static
TARGET = $$qtLibraryTarget(drumstick-rt-mac-in)
DESTDIR = ../../../build/backends
DEPENDPATH += . ../../include
INCLUDEPATH += . ../../include
include (../../../global.pri)
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
QT -= gui

HEADERS += ../../include/rtmidiinput.h \
           ../../include/rtmidioutput.h \
           macmidiinput.h \
           maccommon.h

SOURCES += macmidiinput.cpp \
           maccommon.cpp

LIBS += -framework CoreMidi -framework CoreFoundation

#-framework CoreAudio -framework AudioToolbox -framework AudioUnit
