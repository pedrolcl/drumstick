TEMPLATE = app
TARGET = drumstick-buildsmf
CONFIG += qt console thread exceptions link_prl
DESTDIR = ../../build/bin
INCLUDEPATH += . ../../library/include ../common 
include(../../global.pri)
# Input
HEADERS += buildsmf.h
SOURCES += buildsmf.cpp

macx {
    QMAKE_LFLAGS += -L$$OUT_PWD/../../build/lib/ -F$$OUT_PWD/../../build/lib/
    LIBS += -framework drumstick-file
    LIBS += -l$$qtLibraryTarget(drumstick-common)
}
else {
    LIBS = -L$$OUT_PWD/../../build/lib \
        -l$$qtLibraryTarget(drumstick-common) \
        -l$$qtLibraryTarget(drumstick-file)
}
