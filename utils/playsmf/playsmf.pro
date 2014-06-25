TEMPLATE = app
TARGET = drumstick-playsmf
QT += dbus
CONFIG += qt console thread exceptions link_prl
DESTDIR = ../../build/bin
INCLUDEPATH += . ../../library/include ../common 
LIBS = -L../../build/lib -ldrumstick-common -ldrumstick-alsa -ldrumstick-file -lasound
PRE_TARGETDEPS += $$OUT_PWD/../../build/lib/libdrumstick-common.a
include (../../global.pri)
# Input
HEADERS += playsmf.h
SOURCES += playsmf.cpp
