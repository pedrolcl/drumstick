TEMPLATE = app
TARGET = drumstick-dumpwrk
CONFIG += qt console link_prl
DESTDIR = ../../build/bin
INCLUDEPATH += . ../../library/include ../common 
include (../../global.pri)
# Input
HEADERS += dumpwrk.h
SOURCES += dumpwrk.cpp

LIBS = -L$$OUT_PWD/../../build/lib \
    -l$$qtLibraryTarget(drumstick-common) \
    -l$$qtLibraryTarget(drumstick-file)
