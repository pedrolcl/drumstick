TEMPLATE = app
TARGET = drumstick-drumgrid
INCLUDEPATH += . ../../library/include
DEPENDPATH += . ../../library ../../library/include
SOURCES += main.cpp \
    drumgrid.cpp \
    drumgridmodel.cpp
HEADERS += drumgrid.h \
    drumgridmodel.h
FORMS += drumgrid.ui
LIBS = -L../../library -ldrumstick-alsa  -lasound
