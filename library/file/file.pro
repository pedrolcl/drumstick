TEMPLATE = lib
TARGET = $$qtLibraryTarget(drumstick-file)
DESTDIR = ../../build/lib
DEPENDPATH += . ../include
INCLUDEPATH += . ../include
include (../../global.pri)
CONFIG += qt #create_prl
DEFINES += drumstick_file_EXPORTS
QMAKE_CXXFLAGS += $$QMAKE_CXXFLAGS_HIDESYMS
QT -= gui
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

static {
    DEFINES += DRUMSTICK_STATIC
}

macx {
    TARGET = drumstick-file
    CONFIG += lib_bundle
    FRAMEWORK_HEADERS.version = Versions
    FRAMEWORK_HEADERS.files = $$HEADERS
    FRAMEWORK_HEADERS.path = Headers
    QMAKE_BUNDLE_DATA += FRAMEWORK_HEADERS
    #QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Frameworks/
    QMAKE_SONAME_PREFIX = @rpath
}
