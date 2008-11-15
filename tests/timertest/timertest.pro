TEMPLATE = app
TARGET = aseqmm-timertest
INCLUDEPATH += . ../../library/include
DEPENDPATH += . ../../library ../../library/include
LIBS = -L../../library -laseqmm  -lasound
# Input
SOURCES += timertest.cpp
