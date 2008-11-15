TEMPLATE = app
TARGET = aseqmm-smfplayer
INCLUDEPATH += . ../../library/include
DEPENDPATH += . ../../library ../../library/include
LIBS = -L../../library -laseqmm  -lasound
# Input
HEADERS += player.h smfplayer.h song.h
FORMS += smfplayer.ui
SOURCES += main.cpp player.cpp smfplayer.cpp song.cpp
RESOURCES += smfplayer.qrc
