TEMPLATE = app
TARGET = drumstick-timertest
INCLUDEPATH += . ../../library/include
DEPENDPATH += . ../../library ../../library/include
LIBS = -L../../library -ldrumstick-alsa  -lasound
# Input
HEADERS = timertest.h
SOURCES = timertest.cpp
