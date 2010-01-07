TEMPLATE = app
TARGET = drumstick-vpiano
INCLUDEPATH += . ../../library/include
DEPENDPATH += . ../../library ../../library/include
LIBS = -L../../library -ldrumstick -lasound
QT += svg
# Input
FORMS += vpiano.ui connections.ui about.ui preferences.ui
HEADERS += pianokey.h pianokeybd.h pianoscene.h vpiano.h \
           keyboardmap.h keylabel.h pianodefs.h rawkeybdapp.h \
           connections.h connectionitem.h about.h preferences.h
SOURCES += pianokey.cpp pianokeybd.cpp pianoscene.cpp vpiano.cpp \
           keylabel.cpp rawkeybdapp.cpp \
           connections.cpp about.cpp preferences.cpp main.cpp
RESOURCES += pianokeybd.qrc
