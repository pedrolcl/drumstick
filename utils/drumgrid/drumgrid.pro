TEMPLATE = app
TARGET = drumstick-drumgrid
QT += gui widgets #dbus
CONFIG += c++11
CONFIG += qt thread exceptions
static {
    CONFIG += link_prl
}
DESTDIR = ../../build/bin
INCLUDEPATH += . ../../library/include ../common 
LIBS = -L../../build/lib -ldrumstick-alsa -lasound
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
