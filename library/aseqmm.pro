TEMPLATE = lib
TARGET = aseqmm
DEPENDPATH += . include src
INCLUDEPATH += . include
VERSION=0.0.1
CONFIG += qt staticlib debug
# Input
HEADERS += include/alsatimer.h \
           include/client.h \
           include/commons.h \
           include/event.h \
           include/port.h \
           include/qmidithread.h \
           include/qsmf.h \
           include/queue.h \
           include/subscription.h
SOURCES += src/alsatimer.cpp \
           src/client.cpp \
           src/event.cpp \
           src/port.cpp \
           src/qmidithread.cpp \
           src/qsmf.cpp \
           src/queue.cpp \
           src/subscription.cpp
