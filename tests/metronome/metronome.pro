TEMPLATE = app
TARGET = drumstick-metronome
INCLUDEPATH += . ../../library/include
DEPENDPATH += . ../../library ../../library/include
LIBS = -L../../library -ldrumstick  -lasound
# Input
HEADERS += metronome.h
SOURCES += metronome.cpp
