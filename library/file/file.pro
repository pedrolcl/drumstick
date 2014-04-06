TEMPLATE = lib
TARGET = drumstick-file
DESTDIR = ../../build/lib
OBJECTS_DIR = ../../build/obj
MOC_DIR = ../../build/moc
DEPENDPATH += . ../include
INCLUDEPATH += . ../include
include (../../global.pri)
CONFIG += qt
DEFINES += drumstick_file_EXPORTS
QMAKE_CXXFLAGS += $$QMAKE_CXXFLAGS_HIDESYMS
# Input
HEADERS += ../include/macros.h \
           ../include/qsmf.h \
           ../include/qwrk.h \
           ../include/qove.h
SOURCES += qsmf.cpp \
           qwrk.cpp \
           qove.cpp
win32 {
    TARGET_EXT = .dll
}
