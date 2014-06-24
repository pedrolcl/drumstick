TEMPLATE = app
TARGET = drumstick-vpiano
QT += svg network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets gui
CONFIG += qt thread exceptions link_prl
DESTDIR = ../../build/bin
include (../../global.pri)
INCLUDEPATH += . ../../library/include ../common ../../build/common

# Input
FORMS += vpiano.ui connections.ui vpianoabout.ui preferences.ui
HEADERS += pianokey.h pianokeybd.h pianoscene.h vpiano.h \
           keyboardmap.h keylabel.h pianodefs.h \
           connections.h vpianoabout.h preferences.h
SOURCES += pianokey.cpp pianokeybd.cpp pianoscene.cpp vpiano.cpp \
           keylabel.cpp connections.cpp vpianoabout.cpp preferences.cpp vpianomain.cpp
RESOURCES += pianokeybd.qrc

# libs
LIBS += -L$$OUT_PWD/../../build/lib/
LIBS += -l$$qtLibraryTarget(drumstick-common) \
        -l$$qtLibraryTarget(drumstick-rt-net)


#DEFINES += SYNTH_BACKEND
#LIBS += -l$$qtLibraryTarget(drumstick-rt-synth) \
#        -lfluidsynth

linux* {
    LIBS += -l$$qtLibraryTarget(drumstick-rt-alsa) \
            -l$$qtLibraryTarget(drumstick-alsa) \
            -lasound
}

unix {
    LIBS += -l$$qtLibraryTarget(drumstick-rt-oss)
}

macx {
    LIBS += -l$$qtLibraryTarget(drumstick-rt-mac) \
            -framework CoreMIDI \
            -framework CoreFoundation
}

win32 {
    LIBS += -l$$qtLibraryTarget(drumstick-rt-win) \
            -lwinmm
}
