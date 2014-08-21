TEMPLATE = app
TARGET = drumstick-vpiano
QT += gui widgets svg network
CONFIG += qt thread exceptions link_prl
DESTDIR = ../../build/bin
include (../../global.pri)
INCLUDEPATH += . ../../library/include ../common

# Input
FORMS += vpiano.ui connections.ui vpianoabout.ui preferences.ui fluidsettingsdialog.ui networksettingsdialog.ui
HEADERS += pianokey.h pianokeybd.h pianoscene.h vpiano.h \
           keyboardmap.h keylabel.h pianodefs.h \
           connections.h vpianoabout.h preferences.h \
           fluidsettingsdialog.h networksettingsdialog.h
SOURCES += pianokey.cpp pianokeybd.cpp pianoscene.cpp vpiano.cpp \
           keylabel.cpp connections.cpp vpianoabout.cpp preferences.cpp vpianomain.cpp \
           fluidsettingsdialog.cpp networksettingsdialog.cpp
RESOURCES += pianokeybd.qrc

# libs
macx {
    QMAKE_LFLAGS += -F$$OUT_PWD/../../build/lib -L$$OUT_PWD/../../build/lib
    LIBS += -framework drumstick-rt
    LIBS += -l$$qtLibraryTarget(drumstick-common)
    ICON = ../../icons/drumstick.icns
}
else {
    LIBS += -L$$OUT_PWD/../../build/lib/
    LIBS += -l$$qtLibraryTarget(drumstick-common) \
            -l$$qtLibraryTarget(drumstick-rt)
}

static {
    LIBS += -L$$OUT_PWD/../../build/lib/drumstick/
    LIBS += -l$$qtLibraryTarget(drumstick-rt-net-in) \
            -l$$qtLibraryTarget(drumstick-rt-net-out)

    DEFINES += SYNTH_BACKEND
    CONFIG += link_pkgconfig
    LIBS += -l$$qtLibraryTarget(drumstick-rt-synth)
    PKGCONFIG += fluidsynth

    linux* {
        LIBS += -l$$qtLibraryTarget(drumstick-rt-alsa-in) \
                -l$$qtLibraryTarget(drumstick-rt-alsa-out) \
                -l$$qtLibraryTarget(drumstick-alsa) \
                -lasound
    }

    unix:!macx {
        LIBS += -l$$qtLibraryTarget(drumstick-rt-oss-in) \
                -l$$qtLibraryTarget(drumstick-rt-oss-out)
    }

    macx {
        LIBS += -l$$qtLibraryTarget(drumstick-rt-mac-in) \
                -l$$qtLibraryTarget(drumstick-rt-mac-out) \
                -framework CoreMIDI \
                -framework CoreFoundation
    }

    win32 {
        LIBS += -l$$qtLibraryTarget(drumstick-rt-win-in) \
                -l$$qtLibraryTarget(drumstick-rt-win-out) \
                -lwinmm
    }
}
