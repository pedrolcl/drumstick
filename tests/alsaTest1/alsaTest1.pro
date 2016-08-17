QT       += testlib
QT       -= gui
TARGET = alsatest1
CONFIG   += console
CONFIG   -= app_bundle
TEMPLATE = app
SOURCES += \
    alsatest1.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
INCLUDEPATH += . ../../library/include
LIBS = -L../../build/lib -ldrumstick-alsa -lasound
DESTDIR = ../../build/bin
