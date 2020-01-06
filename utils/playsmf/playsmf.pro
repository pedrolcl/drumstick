TEMPLATE = app
TARGET = drumstick-playsmf
#QT += dbus
CONFIG += c++11 cmdline qt thread exceptions
static {
    CONFIG += link_prl
}
DESTDIR = ../../build/bin
INCLUDEPATH += . ../../library/include ../common 
LIBS = -L../../build/lib -ldrumstick-alsa -ldrumstick-file -lasound
include (../../global.pri)
# Input
HEADERS += playsmf.h
SOURCES += playsmf.cpp
