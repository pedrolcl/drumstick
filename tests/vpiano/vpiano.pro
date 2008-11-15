TEMPLATE = app
TARGET = aseqmm-vpiano
INCLUDEPATH += . ../../library/include
DEPENDPATH += . ../../library ../../library/include
LIBS = -L../../library -laseqmm -lasound
# Input
FORMS += vpiano.ui connections.ui about.ui preferences.ui
HEADERS += pianokey.h pianokeybd.h pianoscene.h vpiano.h \
           connections.h connectionitem.h about.h preferences.h
SOURCES += pianokey.cpp pianokeybd.cpp pianoscene.cpp vpiano.cpp \
           connections.cpp about.cpp preferences.cpp main.cpp
