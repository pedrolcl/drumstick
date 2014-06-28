TEMPLATE = app
TARGET = drumstick-dumpsmf
CONFIG += qt console link_prl
DESTDIR = ../../build/bin
INCLUDEPATH += . ../../library/include ../common 
include (../../global.pri)
# Input
HEADERS += dumpsmf.h
SOURCES += dumpsmf.cpp

LIBS = -L$$OUT_PWD/../../build/lib \
    -l$$qtLibraryTarget(drumstick-common) \
    -l$$qtLibraryTarget(drumstick-file)
