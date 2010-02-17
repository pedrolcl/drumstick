TEMPLATE = app
TARGET = drumstick-testevents
INCLUDEPATH += . ../../library/include
DEPENDPATH += . ../../library ../../library/include
LIBS = -L../../library -ldrumstick-alsa  -lasound
# Input
HEADERS += testevents.h
SOURCES += testevents.cpp
