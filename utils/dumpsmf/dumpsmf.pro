TEMPLATE = app
TARGET = drumstick-dumpsmf
CONFIG += qt console
static {
    CONFIG += link_prl
}
DESTDIR = ../../build/bin
INCLUDEPATH += . ../../library/include ../common 
include (../../global.pri)
# Input
HEADERS += dumpsmf.h
SOURCES += dumpsmf.cpp

macx:!static {
    QMAKE_LFLAGS += -F$$OUT_PWD/../../build/lib -L$$OUT_PWD/../../build/lib
    LIBS += -framework drumstick-file
} else {
    LIBS = -L$$OUT_PWD/../../build/lib \
        -l$$drumstickLib(drumstick-file)
}
