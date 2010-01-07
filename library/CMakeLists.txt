# MIDI Sequencer C++ Library
# Copyright (C) 2005-2009 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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

INCLUDE(CreateLibtoolFile)

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_BINARY_DIR}
)

SET(drumstick_qtobject_SRCS
    include/alsaclient.h
    include/alsaevent.h
    include/alsaport.h
    include/alsaqueue.h
    include/alsatimer.h
    include/playthread.h
    include/qsmf.h
)

SET(drumstick_HEADERS
    include/alsaclient.h
    include/alsaevent.h
    include/alsaport.h
    include/alsaqueue.h
    include/alsatimer.h
    include/drumstickcommon.h
    include/playthread.h
    include/qsmf.h
    include/subscription.h
)

SET(drumstick_SRCS
    src/alsaclient.cpp
    src/alsaevent.cpp
    src/alsaport.cpp
    src/alsaqueue.cpp
    src/alsatimer.cpp
    src/playthread.cpp
    src/qsmf.cpp
    src/subscription.cpp
)

QT4_WRAP_CPP(drumstick_moc_SRCS ${drumstick_qtobject_SRCS})

ADD_LIBRARY(drumstick 
    ${drumstick_moc_SRCS}
    ${drumstick_SRCS} 
    ${drumstick_HEADERS} 
)

TARGET_LINK_LIBRARIES(drumstick
    ${QT_LIBRARIES}
    ${ALSA_LIBS}
)

IF(STATIC_DRUMSTICK)
    SET_TARGET_PROPERTIES(drumstick PROPERTIES
         STATIC_LIB "libdrumstick.a"
         COMPILE_FLAGS -fPIC
    )
ELSE(STATIC_drumstick)
    SET_TARGET_PROPERTIES(drumstick PROPERTIES
        VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
        SOVERSION ${VERSION_MAJOR}
    )
ENDIF(STATIC_DRUMSTICK)

SET_TARGET_PROPERTIES(drumstick PROPERTIES
    LT_DEPENDENCY_LIBS "-lasound -lQtCore"
    LT_VERSION_CURRENT ${VERSION_MAJOR}
    LT_VERSION_AGE ${VERSION_MINOR}
    LT_VERSION_REVISION ${VERSION_PATCH}
)

CREATE_LIBTOOL_FILE(drumstick ${LIB})

# we should install some files...

INSTALL(TARGETS drumstick
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION ${LIB}
	LIBRARY DESTINATION ${LIB})

INSTALL(FILES ${drumstick_HEADERS}
        DESTINATION include/drumstick)

INSTALL(FILES include/drumstick.h
        DESTINATION include)
