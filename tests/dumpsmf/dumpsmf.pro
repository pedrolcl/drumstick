TEMPLATE = app
TARGET = drumstick-dumpsmf
INCLUDEPATH += . ../../library/include
DEPENDPATH += . ../../library ../../library/include
LIBS = -L../../library -ldrumstick-file
# Input
HEADERS += dumpsmf.h
SOURCES += dumpsmf.cpp
