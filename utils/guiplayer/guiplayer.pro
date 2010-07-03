TEMPLATE = app
TARGET = drumstick-guiplayer
DESTDIR = ../../build/bin
OBJECTS_DIR = ../../build/obj
MOC_DIR = ../../build/moc
RCC_DIR = ../../build/rcc
UI_DIR = ../../build/ui
INCLUDEPATH += . ../../library/include ../common ../../build/common
LIBS = -L../../build/lib -ldrumstick-file -ldrumstick-alsa -lasound
# Input
HEADERS += player.h guiplayer.h song.h
FORMS += guiplayer.ui
SOURCES += playermain.cpp \
    player.cpp \
    guiplayer.cpp \
    song.cpp \
    ../common/cmdlineargs.cpp
RESOURCES += guiplayer.qrc
