TEMPLATE = app
TARGET = drumstick-smfplayer
DESTDIR = ../../build/bin
OBJECTS_DIR = ../../build/obj
MOC_DIR = ../../build/moc
RCC_DIR = ../../build/rcc
UI_DIR = ../../build/ui
INCLUDEPATH += . ../../library/include ../common ../../build/common
LIBS = -L../../build/lib -ldrumstick-file -ldrumstick-alsa -lasound
# Input
HEADERS += player.h smfplayer.h song.h
FORMS += smfplayer.ui
SOURCES += main.cpp player.cpp smfplayer.cpp song.cpp ../common/cmdlineargs.cpp
RESOURCES += smfplayer.qrc
