TEMPLATE = lib
TARGET = $$qtLibraryTarget(drumstick-rt)
DESTDIR = ../../build/lib
DEPENDPATH += . ../include
INCLUDEPATH += . ../include
include (../../global.pri)
CONFIG += qt #create_prl
DEFINES += drumstick_rt_EXPORTS
QMAKE_CXXFLAGS += $$QMAKE_CXXFLAGS_HIDESYMS
QT -= gui
# Input
HEADERS += \
    ../include/rtmidiinput.h \
    ../include/rtmidioutput.h \
    ../include/backendmanager.h \
    ../include/macros.h

SOURCES += \
    backendmanager.cpp

win32 {
    TARGET_EXT = .dll
}

macx {
    TARGET = drumstick-rt
    CONFIG += lib_bundle
    FRAMEWORK_HEADERS.version = Versions
    FRAMEWORK_HEADERS.files = $$HEADERS
    FRAMEWORK_HEADERS.path = Headers/drumstick
    QMAKE_BUNDLE_DATA += FRAMEWORK_HEADERS
    #QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Frameworks/
    QMAKE_SONAME_PREFIX = @rpath
}

static {
    LIBS += -L$$OUT_PWD/../../build/lib/drumstick
    LIBS += -L$$OUT_PWD/../../build/lib

    #dummy {
    #    DEFINES += DUMMY_BACKEND
    #    LIBS += -l$$qtLibraryTarget(drumstick-rt-dummy-in) \
    #            -l$$qtLibraryTarget(drumstick-rt-dummy-out)
    #}

    linux* {
        DEFINES += ALSA_BACKEND
        LIBS += -l$$qtLibraryTarget(drumstick-rt-alsa-in) \
                -l$$qtLibraryTarget(drumstick-rt-alsa-out) \
                -l$$qtLibraryTarget(drumstick-alsa) \
                -lasound
    }

    unix:!macx {
        DEFINES += OSS_BACKEND
        LIBS += -l$$qtLibraryTarget(drumstick-rt-oss-in) \
                -l$$qtLibraryTarget(drumstick-rt-oss-out)
    }

    macx {
        DEFINES += MAC_BACKEND
        LIBS += -l$$qtLibraryTarget(drumstick-rt-mac-in) \
                -l$$qtLibraryTarget(drumstick-rt-mac-out) \
                -framework CoreMidi \
                -framework CoreFoundation
    }

    win32 {
        DEFINES += WIN_BACKEND
        LIBS += -l$$qtLibraryTarget(drumstick-rt-win-in) \
                -l$$qtLibraryTarget(drumstick-rt-win-out) \
                -lwinmm
    }

    DEFINES += NET_BACKEND
    QT += network
    LIBS += -l$$qtLibraryTarget(drumstick-rt-net-in) \
            -l$$qtLibraryTarget(drumstick-rt-net-out)

    DEFINES += SYNTH_BACKEND
    CONFIG += link_pkgconfig
    LIBS += -l$$qtLibraryTarget(drumstick-rt-synth)
    PKGCONFIG += fluidsynth
}
