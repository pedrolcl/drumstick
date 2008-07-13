TEMPLATE = app
TARGET = vpiano
DEPENDPATH += .
DEPENDPATH += ../../library
INCLUDEPATH += .
DEPENDPATH +=  ../../library/include
LIBS = -L../../library -laseqmm
# Input
FORMS += vpiano.ui connections.ui about.ui preferences.ui
HEADERS += pianokey.h pianokeybd.h pianoscene.h vpiano.h
SOURCES += main.cpp pianokey.cpp pianokeybd.cpp pianoscene.cpp vpiano.cpp
