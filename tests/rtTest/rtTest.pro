QT       += testlib
QT       -= gui
TARGET = rtTest
CONFIG   += console
CONFIG   -= app_bundle
TEMPLATE = app
SOURCES += rttest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
INCLUDEPATH += . ../../library/include
DESTDIR = ../../build/bin
macx:!static {
    QMAKE_LFLAGS += -F$$OUT_PWD/../../build/lib -L$$OUT_PWD/../../build/lib
    LIBS += -framework drumstick-rt
} else {
    LIBS += -L$$OUT_PWD/../../build/lib \
            -l$$qtLibraryTarget(drumstick-rt)
}
