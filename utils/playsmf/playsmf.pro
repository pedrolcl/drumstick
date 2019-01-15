TEMPLATE = app
TARGET = drumstick-playsmf
#QT += dbus
CONFIG += qt console thread exceptions link_prl
DESTDIR = ../../build/bin
INCLUDEPATH += . ../../library/include ../common 
LIBS = -L../../build/lib -ldrumstick-alsa -ldrumstick-file -lasound
include (../../global.pri)
# Input
HEADERS += playsmf.h
SOURCES += playsmf.cpp
