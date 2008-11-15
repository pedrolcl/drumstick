TEMPLATE = app
TARGET = aseqmm-dumpmid
INCLUDEPATH += . ../../library/include
DEPENDPATH += . ../../library ../../library/include
LIBS = -L../../library -laseqmm  -lasound
# Input
HEADERS += dumpmid.h
SOURCES += dumpmid.cpp
