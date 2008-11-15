TEMPLATE = app
TARGET = aseqmm-playsmf
INCLUDEPATH += . ../../library/include
DEPENDPATH += . ../../library ../../library/include
LIBS = -L../../library -laseqmm  -lasound
# Input
HEADERS += playsmf.h
SOURCES += playsmf.cpp
