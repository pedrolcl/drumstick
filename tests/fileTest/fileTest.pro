QT       += testlib
QT       -= gui
TARGET = fileTest
CONFIG   += console
CONFIG   -= app_bundle
TEMPLATE = app
SOURCES += \
    filetest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
INCLUDEPATH += . ../../library/include
LIBS += -L$$OUT_PWD/../../build/lib \
        -l$$qtLibraryTarget(drumstick-file)
DESTDIR = ../../build/bin
