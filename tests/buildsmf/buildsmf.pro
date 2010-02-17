TEMPLATE = app
TARGET = drumstick-buildsmf
INCLUDEPATH += . ../../library/include
DEPENDPATH += . ../../library ../../library/include
LIBS = -L../../library -ldrumstick-file
# Input
HEADERS += buildsmf.h
SOURCES += buildsmf.cpp
