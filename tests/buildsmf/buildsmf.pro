TEMPLATE = app
TARGET = drumstick-buildsmf
INCLUDEPATH += . ../../library/include
DEPENDPATH += . ../../library ../../library/include
LIBS = -L../../library -ldrumstick
# Input
HEADERS += buildsmf.h
SOURCES += buildsmf.cpp
