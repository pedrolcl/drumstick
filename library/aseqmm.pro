TEMPLATE = lib
TARGET = aseqmm
DEPENDPATH += . include src
INCLUDEPATH += . include
VERSION=0.0.2
CONFIG += qt staticlib debug

# Input
HEADERS += include/alsatimer.h \
           include/aseqmm.h \
           include/client.h \
           include/commons.h \
           include/event.h \
           include/playthread.h \
           include/port.h \
           include/qsmf.h \
           include/queue.h \
           include/recthread.h \
           include/subscription.h
	   
SOURCES += src/alsatimer.cpp \
           src/client.cpp \
           src/event.cpp \
           src/port.cpp \
           src/playthread.cpp \
           src/qsmf.cpp \
           src/queue.cpp \
           src/recthread.cpp \
           src/subscription.cpp
