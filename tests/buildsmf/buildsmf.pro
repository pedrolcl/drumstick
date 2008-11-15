TEMPLATE = app
TARGET = aseqmm-buildsmf
INCLUDEPATH += . ../../library/include
DEPENDPATH += . ../../library ../../library/include
LIBS = -L../../library -laseqmm
# Input
HEADERS += buildsmf.h
SOURCES += buildsmf.cpp
