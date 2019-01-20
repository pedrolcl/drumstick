TEMPLATE = app
TARGET = drumstick-vpiano
QT += gui widgets svg network
CONFIG += qt thread exceptions
DESTDIR = ../../build/bin
include (../../global.pri)
INCLUDEPATH += . ../../library/include ../common

# Input
FORMS += vpiano.ui connections.ui vpianoabout.ui preferences.ui fluidsettingsdialog.ui networksettingsdialog.ui \
    sonivoxsettingsdialog.ui \
    macsynthsettingsdialog.ui
HEADERS += pianokey.h pianokeybd.h pianoscene.h vpiano.h \
    keyboardmap.h keylabel.h pianodefs.h \
    connections.h vpianoabout.h preferences.h \
    fluidsettingsdialog.h networksettingsdialog.h \
    macsynthsettingsdialog.h \
    sonivoxsettingsdialog.h \
    ../../library/include/rtmidiinput.h
SOURCES += pianokey.cpp pianokeybd.cpp pianoscene.cpp vpiano.cpp \
    keylabel.cpp connections.cpp vpianoabout.cpp preferences.cpp vpianomain.cpp \
    fluidsettingsdialog.cpp networksettingsdialog.cpp \
    macsynthsettingsdialog.cpp \
    sonivoxsettingsdialog.cpp
RESOURCES += pianokeybd.qrc

# libs
macx:!static {
    QMAKE_LFLAGS += -F$$OUT_PWD/../../build/lib -L$$OUT_PWD/../../build/lib
    LIBS += -framework drumstick-rt
    ICON = ../../icons/drumstick.icns
    QMAKE_TARGET_BUNDLE_PREFIX = net.sourceforge
    QMAKE_BUNDLE = drumstick-vpiano
    QMAKE_INFO_PLIST = ../Info.plist.app
} else {
    LIBS += -L$$OUT_PWD/../../build/lib/
    LIBS += -l$$drumstickLib(drumstick-rt)
}

static {
    CONFIG += link_prl

    LIBS += -L$$OUT_PWD/../../build/lib/drumstick/
    LIBS += -ldrumstick-rt-net-in \
            -ldrumstick-rt-net-out

    packagesExist(fluidsynth) {
        DEFINES += SYNTH_BACKEND
        LIBS += -ldrumstick-rt-synth
        macx {
            QMAKE_LFLAGS += -F/Library/Frameworks
            LIBS += -framework FluidSynth
        } else {
            CONFIG += link_pkgconfig
            PKGCONFIG += fluidsynth
        }
    }

    linux* {
        LIBS += -ldrumstick-rt-alsa-in \
                -ldrumstick-rt-alsa-out \
                -ldrumstick-rt-eassynth \
                -ldrumstick-alsa \
                -lasound
    }

    unix:!macx {
        LIBS += -ldrumstick-rt-oss-in \
                -ldrumstick-rt-oss-out
    }

    macx {
        LIBS += -ldrumstick-rt-mac-in \
                -ldrumstick-rt-mac-out \
                -ldrumstick-rt-macsynth \
                -framework CoreMIDI \
                -framework CoreFoundation
    }

    win32 {
        LIBS += -ldrumstick-rt-win-in \
                -ldrumstick-rt-win-out \
                -lwinmm
    }
}
