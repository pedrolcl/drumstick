TEMPLATE = app
TARGET = aseqmm-testevents
INCLUDEPATH += . ../../library/include
DEPENDPATH += . ../../library ../../library/include
LIBS = -L../../library -laseqmm  -lasound
# Input
HEADERS += testevents.h
SOURCES += testevents.cpp
