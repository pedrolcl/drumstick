TEMPLATE = lib
TARGET = drumstick-file
DESTDIR = ../build/lib
OBJECTS_DIR = ../build/obj
MOC_DIR = ../build/moc
DEPENDPATH += . include src
INCLUDEPATH += . include
include (../global.pri)
CONFIG += qt staticlib debug
DEFINES += drumstick_file_EXPORTS
# Input
HEADERS += include/macros.h
           include/qsmf.h \
           include/qwrk.h \
           include/qove.h
SOURCES += src/qsmf.cpp \
           src/qwrk.cpp \
           src/qove.cpp
