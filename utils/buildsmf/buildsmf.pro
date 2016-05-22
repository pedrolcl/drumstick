TEMPLATE = app
TARGET = drumstick-buildsmf
CONFIG += qt console thread exceptions link_prl
DESTDIR = ../../build/bin
INCLUDEPATH += . ../../library/include ../common 
include(../../global.pri)
# Input
HEADERS += buildsmf.h
SOURCES += buildsmf.cpp

macx:!static {
    QMAKE_LFLAGS += -L$$OUT_PWD/../../build/lib/ -F$$OUT_PWD/../../build/lib/
    LIBS += -l$$qtLibraryTarget(drumstick-common)
    LIBS += -framework drumstick-file
} else {
    LIBS = -L$$OUT_PWD/../../build/lib \
        -l$$qtLibraryTarget(drumstick-common) \
        -l$$qtLibraryTarget(drumstick-file)
}
