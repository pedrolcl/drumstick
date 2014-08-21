TEMPLATE = app
TARGET = drumstick-dumpove
CONFIG += qt console link_prl
DESTDIR = ../../build/bin
INCLUDEPATH += . ../../library/include ../common 
include (../../global.pri)
# Input
HEADERS += dumpove.h
SOURCES += dumpove.cpp

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
