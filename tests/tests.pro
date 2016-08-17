QT       += testlib
QT       -= gui
TARGET = drumsticktest
CONFIG   += console
CONFIG   -= app_bundle
TEMPLATE = app
SOURCES += drumsticktest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
