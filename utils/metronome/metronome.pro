TEMPLATE = app
TARGET = drumstick-metronome
#QT += dbus
CONFIG += c++11 cmdline qt thread exceptions
static {
    CONFIG += link_prl
}
DESTDIR = ../../build/bin
INCLUDEPATH += . ../../library/include
LIBS = -L../../build/lib -ldrumstick-alsa -lasound
include (../../global.pri)
# Input
HEADERS += metronome.h
SOURCES += metronome.cpp
