TEMPLATE = app
TARGET = drumstick-sysinfo
INCLUDEPATH += . ../../library/include
DEPENDPATH += . ../../library ../../library/include
LIBS = -L../../library -ldrumstick  -lasound
# Input
SOURCES += sysinfo.cpp
