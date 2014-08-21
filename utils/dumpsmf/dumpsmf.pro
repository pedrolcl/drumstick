TEMPLATE = app
TARGET = drumstick-dumpsmf
CONFIG += qt console link_prl
DESTDIR = ../../build/bin
INCLUDEPATH += . ../../library/include ../common 
include (../../global.pri)
# Input
HEADERS += dumpsmf.h
SOURCES += dumpsmf.cpp

macx {
    QMAKE_LFLAGS += -F$$OUT_PWD/../../build/lib -L$$OUT_PWD/../../build/lib
    LIBS += -framework drumstick-file
    LIBS += -l$$qtLibraryTarget(drumstick-common)
}
else {
    LIBS = -L$$OUT_PWD/../../build/lib \
        -l$$qtLibraryTarget(drumstick-common) \
        -l$$qtLibraryTarget(drumstick-file)
}
