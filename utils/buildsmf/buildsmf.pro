TEMPLATE = app
TARGET = drumstick-buildsmf
CONFIG += qt console thread exceptions link_prl
DESTDIR = ../../build/bin
INCLUDEPATH += . ../../library/include ../common 
include(../../global.pri)
# Input
HEADERS += buildsmf.h
SOURCES += buildsmf.cpp

LIBS = -L$$OUT_PWD/../../build/lib \
    -l$$qtLibraryTarget(drumstick-common) \
    -l$$qtLibraryTarget(drumstick-file)

