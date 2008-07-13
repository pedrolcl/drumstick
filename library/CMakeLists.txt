# MIDI Sequencer C++ Library
# Copyright (C) 2005-2008 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA 02110-1301, USA

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_BINARY_DIR}
)

SET(aseqmm_qtobject_SRCS
    include/client.h
    include/port.h
    include/qsmf.h
    include/qmidithread.h
)

SET(aseqmm_SRCS
    include/client.h
    include/commons.h
    include/event.h
    include/port.h
    include/qmidithread.h
    include/qsmf.h
    include/queue.h
    include/subscription.h
    include/alsatimer.h
    src/client.cpp
    src/event.cpp
    src/port.cpp
    src/qmidithread.cpp
    src/qsmf.cpp
    src/queue.cpp
    src/subscription.cpp
    src/alsatimer.cpp
)

QT4_WRAP_CPP(aseqmm_moc_SRCS ${aseqmm_qtobject_SRCS})

ADD_LIBRARY(aseqmm 
    ${aseqmm_moc_SRCS}
    ${aseqmm_SRCS} 
)

TARGET_LINK_LIBRARIES(aseqmm
    ${QT_LIBRARIES}
    ${ALSA_LIBS}
)

IF(NOT WANT_STATIC)
    SET_TARGET_PROPERTIES(aseqmm PROPERTIES
        VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
        SOVERSION ${VERSION_MAJOR}
    )
ENDIF(NOT WANT_STATIC)

# we should install some files...
INSTALL(TARGETS aseqmm
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib)
