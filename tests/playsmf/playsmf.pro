TEMPLATE = app
TARGET = drumstick-playsmf
INCLUDEPATH += . ../../library/include
DEPENDPATH += . ../../library ../../library/include
LIBS = -L../../library -ldrumstick  -lasound
# Input
HEADERS += playsmf.h
SOURCES += playsmf.cpp
