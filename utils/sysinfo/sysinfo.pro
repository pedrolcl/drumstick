TEMPLATE = app
TARGET = drumstick-sysinfo
#QT += dbus
CONFIG += c++11 cmdline qt thread exceptions
static {
    CONFIG += link_prl
}
DESTDIR = ../../build/bin
INCLUDEPATH += . ../../library/include ../common 
DEPENDPATH += . ../../library ../../library/include
LIBS = -L../../build/lib -ldrumstick-alsa -lasound
include (../../global.pri)
# Input
SOURCES += sysinfo.cpp
