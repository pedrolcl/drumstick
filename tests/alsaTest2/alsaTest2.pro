QT       += testlib
QT       -= gui
TARGET = alsaTest2
CONFIG   += c++11 cmdline
TEMPLATE = app
SOURCES += \
    alsatest2.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
INCLUDEPATH += . ../../library/include
LIBS = -L../../build/lib -ldrumstick-alsa -lasound
DESTDIR = ../../build/bin
