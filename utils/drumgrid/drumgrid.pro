TEMPLATE = app
TARGET = drumstick-drumgrid
QT += gui widgets #dbus
CONFIG += c++11
CONFIG += qt thread exceptions
CONFIG += lrelease
static {
    CONFIG += link_prl
}
DESTDIR = ../../build/bin
LRELEASE_DIR=.
INCLUDEPATH += . ../../library/include
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
TRANSLATIONS += \
    drumstick-drumgrid_en.ts \
    drumstick-drumgrid_es.ts
