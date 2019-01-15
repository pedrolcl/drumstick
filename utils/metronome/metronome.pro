TEMPLATE = app
TARGET = drumstick-metronome
#QT += dbus
CONFIG += qt console thread exceptions link_prl
DESTDIR = ../../build/bin
INCLUDEPATH += . ../../library/include ../common 
LIBS = -L../../build/lib -ldrumstick-alsa -lasound
include (../../global.pri)
# Input
HEADERS += metronome.h
SOURCES += metronome.cpp
