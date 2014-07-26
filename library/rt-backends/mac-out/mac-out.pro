TEMPLATE = lib
CONFIG += plugin #create_prl #static
TARGET = $$qtLibraryTarget(drumstick-rt-mac-out)
DESTDIR = ../../../build/lib/drumstick
DEPENDPATH += . ../../include ../common
INCLUDEPATH += . ../../include ../common
include (../../../global.pri)
QT -= gui

HEADERS += ../../include/rtmidiinput.h \
           ../../include/rtmidioutput.h \
           macmidioutput.h \
           ../common/maccommon.h

SOURCES += \
           macmidioutput.cpp \
           ../common/maccommon.cpp

LIBS += -framework CoreMidi -framework CoreFoundation

#-framework CoreAudio -framework AudioToolbox -framework AudioUnit
