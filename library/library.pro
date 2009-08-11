TEMPLATE = lib
TARGET = aseqmm
DEPENDPATH += . include src
INCLUDEPATH += . include
VERSION=0.0.2
CONFIG += qt staticlib debug

# Input
HEADERS += include/alsaclient.h \
           include/alsaevent.h \
           include/alsaport.h \
           include/alsaqueue.h \
           include/alsatimer.h \
           include/aseqmm.h \
           include/aseqmmcommon.h \
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
