TEMPLATE = app
TARGET = drumstick-drumgrid
DESTDIR = ../../build/bin
OBJECTS_DIR = ../../build/obj
MOC_DIR = ../../build/moc
UI_DIR = ../../build/ui
INCLUDEPATH += . ../../library/include ../common ../../build/common
LIBS = -L../../build/lib -ldrumstick-alsa  -lasound
SOURCES += drumgridmain.cpp \
    drumgrid.cpp \
    drumgridmodel.cpp \
    ../common/cmdlineargs.cpp
HEADERS += drumgrid.h \
    drumgridmodel.h
FORMS += drumgrid.ui
