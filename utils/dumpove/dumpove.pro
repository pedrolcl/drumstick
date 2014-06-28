TEMPLATE = app
TARGET = drumstick-dumpove
CONFIG += qt console link_prl
DESTDIR = ../../build/bin
INCLUDEPATH += . ../../library/include ../common 
include (../../global.pri)
# Input
HEADERS += dumpove.h
SOURCES += dumpove.cpp

LIBS = -L$$OUT_PWD/../../build/lib \
    -l$$qtLibraryTarget(drumstick-common) \
    -l$$qtLibraryTarget(drumstick-file)
