TEMPLATE = app
TARGET = drumstick-buildsmf
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += qt console thread exceptions link_prl
DESTDIR = ../../build/bin
INCLUDEPATH += . ../../library/include ../common 
LIBS = -L$$OUT_PWD/../../build/lib -ldrumstick-common -ldrumstick-file
PRE_TARGETDEPS += $$OUT_PWD/../../build/lib/libdrumstick-common.a

include (../../global.pri)
# Input
HEADERS += buildsmf.h
SOURCES += buildsmf.cpp
