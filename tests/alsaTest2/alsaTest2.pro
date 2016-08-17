QT       += testlib
QT       -= gui
TARGET = alsaTest2
CONFIG   += console
CONFIG   -= app_bundle
TEMPLATE = app
SOURCES += \
    alsatest2.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
INCLUDEPATH += . ../../library/include
LIBS = -L../../build/lib -ldrumstick-alsa -lasound
DESTDIR = ../../build/bin
