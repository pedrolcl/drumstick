TEMPLATE = app
TARGET = drumstick-smfplayer
INCLUDEPATH += . ../../library/include
DEPENDPATH += . ../../library ../../library/include
LIBS = -L../../library -ldrumstick-file -ldrumstick-alsa -lasound
# Input
HEADERS += player.h smfplayer.h song.h
FORMS += smfplayer.ui
SOURCES += main.cpp player.cpp smfplayer.cpp song.cpp
RESOURCES += smfplayer.qrc
