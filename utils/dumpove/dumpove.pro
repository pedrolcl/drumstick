TEMPLATE = app
TARGET = drumstick-dumpove
CONFIG += c++11 cmdline qt
static {
    CONFIG += link_prl
    DEFINES += DRUMSTICK_STATIC
}
DESTDIR = ../../build/bin
INCLUDEPATH += . ../../library/include ../common 
include (../../global.pri)
# Input
HEADERS += dumpove.h
SOURCES += dumpove.cpp

macx:!static {
    QMAKE_LFLAGS += -F$$OUT_PWD/../../build/lib -L$$OUT_PWD/../../build/lib
    LIBS += -framework drumstick-file
} else {
    LIBS = -L$$OUT_PWD/../../build/lib \
        -l$$drumstickLib(drumstick-file)
}
