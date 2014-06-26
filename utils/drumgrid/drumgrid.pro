TEMPLATE = app
TARGET = drumstick-drumgrid
QT += gui widgets dbus
CONFIG += qt thread exceptions link_prl
DESTDIR = ../../build/bin
INCLUDEPATH += . ../../library/include ../common 
LIBS = -L../../build/lib -ldrumstick-common -ldrumstick-alsa -lasound
PRE_TARGETDEPS += $$OUT_PWD/../../build/lib/libdrumstick-common.a
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
