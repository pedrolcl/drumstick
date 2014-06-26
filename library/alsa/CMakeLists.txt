# MIDI Sequencer C++ Library
# Copyright (C) 2005-2014 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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

#INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )

set(CMAKE_INCLUDE_CURRENT_DIR ON)

SET(drumstick-alsa_QTOBJ_SRCS
    ../include/alsaclient.h
    ../include/alsaevent.h
    ../include/alsaport.h
    ../include/alsaqueue.h
    ../include/alsatimer.h
    ../include/playthread.h
)

SET(drumstick-alsa_HEADERS
    ../include/alsaclient.h
    ../include/alsaevent.h
    ../include/alsaport.h
    ../include/alsaqueue.h
    ../include/alsatimer.h
    ../include/drumstickcommon.h
    ../include/playthread.h
    ../include/subscription.h
)

SET(drumstick-alsa_SRCS
    alsaclient.cpp
    alsaevent.cpp
    alsaport.cpp
    alsaqueue.cpp
    alsatimer.cpp
    playthread.cpp
    subscription.cpp
)

QT5_WRAP_CPP(drumstick-alsa_MOC_SRCS ${drumstick-alsa_QTOBJ_SRCS})

ADD_LIBRARY(drumstick-alsa
    ${drumstick-alsa_MOC_SRCS}
    ${drumstick-alsa_SRCS}
    ${drumstick-alsa_HEADERS}
)

TARGET_LINK_LIBRARIES(drumstick-alsa
    ${ALSA_LIBS}
)

IF(STATIC_DRUMSTICK)
    SET_TARGET_PROPERTIES(drumstick-alsa PROPERTIES
         STATIC_LIB "libdrumstick-alsa.a"
    )
    IF(NOT WIN32)
        SET_TARGET_PROPERTIES(drumstick-file PROPERTIES
        COMPILE_FLAGS -fPIC
    )
    ENDIF(NOT WIN32)
ELSE(STATIC_DRUMSTICK)
    SET_TARGET_PROPERTIES(drumstick-alsa PROPERTIES
        VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
        SOVERSION ${VERSION_MAJOR}
    )
ENDIF(STATIC_DRUMSTICK)

INSTALL(TARGETS drumstick-alsa
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib${LIB_SUFFIX}
    LIBRARY DESTINATION lib${LIB_SUFFIX})
INSTALL(FILES ${drumstick-alsa_HEADERS}
        DESTINATION include/drumstick)
INSTALL(FILES include/drumstick.h
        DESTINATION include)
            
