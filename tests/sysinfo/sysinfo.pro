TEMPLATE = app
TARGET = aseqmm-sysinfo
INCLUDEPATH += . ../../library/include
DEPENDPATH += . ../../library ../../library/include
LIBS = -L../../library -laseqmm  -lasound
# Input
SOURCES += sysinfo.cpp
