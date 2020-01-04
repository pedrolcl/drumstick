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

static {
    LIBS += -L$$OUT_PWD/../../build/lib/drumstick
    LIBS += -L$$OUT_PWD/../../build/lib

    #dummy {
    #    DEFINES += DUMMY_BACKEND
    #    LIBS += -ldrumstick-rt-dummy-in \
    #            -ldrumstick-rt-dummy-out
    #}

    linux* {
        DEFINES += LINUX_BACKEND
        LIBS += -ldrumstick-rt-alsa-in \
                -ldrumstick-rt-alsa-out \
                -ldrumstick-alsa \
                -lasound
        packagesExist(libpulse-simple) {
            LIBS += -ldrumstick-rt-eassynth
            CONFIG += link_pkgconfig
            PKGCONFIG += libpulse-simple
        }
    }

    unix:!macx {
        DEFINES += OSS_BACKEND
        LIBS += -ldrumstick-rt-oss-in \
                -ldrumstick-rt-oss-out
    }

    macx {
        DEFINES += MAC_BACKEND
        LIBS += -ldrumstick-rt-mac-in \
                -ldrumstick-rt-mac-out \
                -ldrumstick-rt-macsynth \
                -framework CoreMIDI \
                -framework CoreFoundation \
                -framework CoreServices \
                -framework CoreAudio \
                -framework AudioToolbox \
                -framework AudioUnit
    }

    win32 {
        DEFINES += WIN_BACKEND
        LIBS += -ldrumstick-rt-win-in \
                -ldrumstick-rt-win-out \
                -lwinmm
    }

    DEFINES += NET_BACKEND
    QT += network
    LIBS += -ldrumstick-rt-net-in \
            -ldrumstick-rt-net-out

    packagesExist(fluidsynth) {
        DEFINES += FLUIDSYNTH_BACKEND
        LIBS += -ldrumstick-rt-fluidsynth
        macx {
            LIBS += -framework FluidSynth
        } else {
            CONFIG += link_pkgconfig
            PKGCONFIG += fluidsynth
        }
    }
}
