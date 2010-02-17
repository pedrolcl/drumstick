TEMPLATE = app
TARGET = drumstick-sysinfo
INCLUDEPATH += . ../../library/include
DEPENDPATH += . ../../library ../../library/include
LIBS = -L../../library -ldrumstick-alsa  -lasound
# Input
SOURCES += sysinfo.cpp
