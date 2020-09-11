TEMPLATE  = app
TARGET    = widgetsTest
QT       += testlib gui widgets
CONFIG   += c++11 cmdline
include (../../global.pri)
SOURCES += \
    widgetstest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
INCLUDEPATH += . ../../library/include/
DESTDIR = ../../build/bin

macx:!static {
    QMAKE_LFLAGS += -F$$OUT_PWD/../../build/lib -L$$OUT_PWD/../../build/lib
    LIBS += -framework drumstick-widgets
} else {
    LIBS += -L$$OUT_PWD/../../build/lib \
            -l$$drumstickLib(drumstick-widgets)
}

