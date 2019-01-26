TEMPLATE = lib
TARGET = drumstick-file
DESTDIR = ../../build/lib
DEPENDPATH += . ../include
INCLUDEPATH += . ../include
include (../../global.pri)
CONFIG += qt
DEFINES += drumstick_file_EXPORTS
QMAKE_CXXFLAGS += $$QMAKE_CXXFLAGS_HIDESYMS
QT -= gui
# Input
HEADERS += ../include/drumstick/macros.h \
           ../include/drumstick/qsmf.h \
           ../include/drumstick/qwrk.h \
           ../include/drumstick/qove.h
SOURCES += qsmf.cpp \
           qwrk.cpp \
           qove.cpp

static {
    CONFIG += staticlib create_prl
    DEFINES += DRUMSTICK_STATIC
}

macx:!static {
    TARGET = drumstick-file
    CONFIG += lib_bundle
    FRAMEWORK_HEADERS.version = Versions
    FRAMEWORK_HEADERS.files = $$HEADERS
    FRAMEWORK_HEADERS.path = Headers/drumstick
    QMAKE_BUNDLE_DATA += FRAMEWORK_HEADERS
    #QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Frameworks/
    QMAKE_SONAME_PREFIX = @rpath
    QMAKE_TARGET_BUNDLE_PREFIX = net.sourceforge
    QMAKE_BUNDLE = drumstick-file
    QMAKE_INFO_PLIST = ../Info.plist.lib
}
