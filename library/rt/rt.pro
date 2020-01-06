TEMPLATE = lib
TARGET = drumstick-rt
DESTDIR = ../../build/lib
DEPENDPATH += . ../include
INCLUDEPATH += . ../include
include (../../global.pri)
CONFIG += qt create_pc create_prl no_install_prl
static {
    CONFIG += staticlib
}
DEFINES += drumstick_rt_EXPORTS
QMAKE_CXXFLAGS += $$QMAKE_CXXFLAGS_HIDESYMS
QMAKE_PKGCONFIG_PREFIX = $$INSTALLBASE
QT -= gui
# Input
HEADERS += \
    ../include/drumstick/rtmidiinput.h \
    ../include/drumstick/rtmidioutput.h \
    ../include/drumstick/backendmanager.h \
    ../include/drumstick/macros.h

SOURCES += \
    backendmanager.cpp

macx:!static {
    TARGET = drumstick-rt
    CONFIG += lib_bundle
    FRAMEWORK_HEADERS.version = Versions
    FRAMEWORK_HEADERS.files = $$HEADERS
    FRAMEWORK_HEADERS.path = Headers/drumstick
    QMAKE_BUNDLE_DATA += FRAMEWORK_HEADERS
    #QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Frameworks/
    QMAKE_SONAME_PREFIX = @rpath
    QMAKE_TARGET_BUNDLE_PREFIX = net.sourceforge
    QMAKE_BUNDLE = drumstick-rt
    QMAKE_INFO_PLIST = ../Info.plist.lib
}
