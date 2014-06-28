TEMPLATE = lib
CONFIG += plugin #create_prl #static
TARGET = $$qtLibraryTarget(drumstick-rt-mac-out)
DESTDIR = ../../../build/backends
DEPENDPATH += . ../../include
INCLUDEPATH += . ../../include
include (../../../global.pri)
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
QT -= gui

HEADERS += ../../include/rtmidiinput.h \
           ../../include/rtmidioutput.h \
           macmidioutput.h \
           maccommon.h

SOURCES += \
           macmidioutput.cpp \
           maccommon.cpp

LIBS += -framework CoreMidi -framework CoreFoundation

#-framework CoreAudio -framework AudioToolbox -framework AudioUnit
