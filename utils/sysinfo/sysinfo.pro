TEMPLATE = app
TARGET = drumstick-sysinfo
#QT += dbus
CONFIG += qt console thread exceptions link_prl
DESTDIR = ../../build/bin
INCLUDEPATH += . ../../library/include ../common 
DEPENDPATH += . ../../library ../../library/include
LIBS = -L../../build/lib -ldrumstick-alsa -lasound
include (../../global.pri)
# Input
SOURCES += sysinfo.cpp
