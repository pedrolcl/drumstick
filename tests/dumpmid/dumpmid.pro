TEMPLATE = app
TARGET = drumstick-dumpmid
INCLUDEPATH += . ../../library/include
DEPENDPATH += . ../../library ../../library/include
LIBS = -L../../library -ldrumstick-alsa  -lasound
# Input
HEADERS += dumpmid.h
SOURCES += dumpmid.cpp
