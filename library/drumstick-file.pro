TEMPLATE = lib
TARGET = drumstick-file
DESTDIR = ../build/lib
OBJECTS_DIR = ../build/obj
MOC_DIR = ../build/moc
DEPENDPATH += . include src
INCLUDEPATH += . include
VERSION=0.3.2svn
CONFIG += qt staticlib debug
# Input
HEADERS += include/qsmf.h \
           include/qwrk.h
SOURCES += src/qsmf.cpp \
           src/qwrk.cpp
