TEMPLATE = app
TARGET = drumstick-drumgrid
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets gui
QT += dbus
CONFIG += qt thread exceptions
DESTDIR = ../../build/bin
OBJECTS_DIR = ../../build/obj
MOC_DIR = ../../build/moc
UI_DIR = ../../build/ui
INCLUDEPATH += . ../../library/include ../common ../../build/common
LIBS = -L../../build/lib -ldrumstick-common -ldrumstick-alsa -lasound
include (../../global.pri)
SOURCES += drumgridabout.cpp \
    drumgridmain.cpp \
    drumgrid.cpp \
    drumgridmodel.cpp 
HEADERS += drumgridabout.h \
    drumgrid.h \
    drumgridmodel.h
FORMS += drumgridabout.ui \
    drumgrid.ui
