TEMPLATE = lib
TARGET = drumstick
DEPENDPATH += . include src
INCLUDEPATH += . include
VERSION=0.2.99svn
CONFIG += qt staticlib debug
# Input
HEADERS += include/alsaclient.h \
           include/alsaevent.h \
           include/alsaport.h \
           include/alsaqueue.h \
           include/alsatimer.h \
           include/drumstick.h \
           include/drumstickcommon.h \
           include/playthread.h \
           include/qsmf.h \
           include/subscription.h
	   
SOURCES += src/alsaclient.cpp \
           src/alsaevent.cpp \
           src/alsaport.cpp \
           src/alsaqueue.cpp \
           src/alsatimer.cpp \
           src/playthread.cpp \
           src/qsmf.cpp \
           src/subscription.cpp
