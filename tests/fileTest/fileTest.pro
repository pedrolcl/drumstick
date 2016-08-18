QT       += testlib
QT       -= gui
TARGET = fileTest
CONFIG   += console
CONFIG   -= app_bundle
TEMPLATE = app
SOURCES += filetest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
INCLUDEPATH += . ../../library/include
DESTDIR = ../../build/bin
macx:!static {
    QMAKE_LFLAGS += -F$$OUT_PWD/../../build/lib -L$$OUT_PWD/../../build/lib
    LIBS += -framework drumstick-file
} else {
    LIBS += -L$$OUT_PWD/../../build/lib \
            -l$$qtLibraryTarget(drumstick-file)
}

