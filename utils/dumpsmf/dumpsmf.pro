TEMPLATE = app
TARGET = drumstick-dumpsmf
CONFIG += c++11 cmdline qt
equals(QT_MAJOR_VERSION, 6) {
    QT += core5compat
}
static {
    CONFIG += link_prl
    DEFINES += DRUMSTICK_STATIC
}
DESTDIR = ../../build/bin
INCLUDEPATH += . ../../library/include
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
