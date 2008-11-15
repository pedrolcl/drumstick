TEMPLATE = app
TARGET = aseqmm-dumpsmf
INCLUDEPATH += . ../../library/include
DEPENDPATH += . ../../library ../../library/include
LIBS = -L../../library -laseqmm
# Input
HEADERS += dumpsmf.h
SOURCES += dumpsmf.cpp
