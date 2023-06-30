TEMPLATE = app
TARGET = drumstick-dumprmi
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
HEADERS += dumprmi.h riff.h
SOURCES += dumprmi.cpp main.cpp riff.cpp

macx:!static {
    QMAKE_LFLAGS += -F$$OUT_PWD/../../build/lib -L$$OUT_PWD/../../build/lib
    LIBS += -framework drumstick-file
} else {
    LIBS = -L$$OUT_PWD/../../build/lib \
        -l$$drumstickLib(drumstick-file)
}
