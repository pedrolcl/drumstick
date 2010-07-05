TEMPLATE = app
TARGET = drumstick-vpiano
DESTDIR = ../../build/bin
OBJECTS_DIR = ../../build/obj
MOC_DIR = ../../build/moc
RCC_DIR = ../../build/rcc
UI_DIR = ../../build/ui
INCLUDEPATH += . ../../library/include ../common ../../build/common
LIBS = -L../../build/lib -ldrumstick-common -ldrumstick-alsa -lasound
QT += svg
# Input
FORMS += vpiano.ui connections.ui vpianoabout.ui preferences.ui
HEADERS += pianokey.h pianokeybd.h pianoscene.h vpiano.h \
           keyboardmap.h keylabel.h pianodefs.h rawkeybdapp.h \
           connections.h connectionitem.h vpianoabout.h preferences.h
SOURCES += pianokey.cpp pianokeybd.cpp pianoscene.cpp vpiano.cpp \
           keylabel.cpp rawkeybdapp.cpp \
           connections.cpp vpianoabout.cpp preferences.cpp vpianomain.cpp
RESOURCES += pianokeybd.qrc
