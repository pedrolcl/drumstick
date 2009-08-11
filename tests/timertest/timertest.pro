TEMPLATE = app
TARGET = aseqmm-timertest
INCLUDEPATH += . ../../library/include
DEPENDPATH += . ../../library ../../library/include
LIBS = -L../../library -laseqmm  -lasound
# Input
HEADERS = timertest.h
SOURCES = timertest.cpp
