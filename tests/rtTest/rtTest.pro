QT       += testlib
QT       -= gui
TARGET = rtTest
CONFIG   += console
CONFIG   -= app_bundle
TEMPLATE = app
SOURCES += \
    rttest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
LIBS += -L$$OUT_PWD/../../build/lib \
        -l$$qtLibraryTarget(drumstick-rt)
INCLUDEPATH += . ../../library/include
DESTDIR = ../../build/bin
