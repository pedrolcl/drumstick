TEMPLATE = app
TARGET = drumstick-dumpwrk
INCLUDEPATH += . ../../library/include
DEPENDPATH += . ../../library ../../library/include
LIBS = -L../../library -ldrumstick-file
# Input
HEADERS += dumpwrk.h
SOURCES += dumpwrk.cpp
