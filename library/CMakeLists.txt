# MIDI Sequencer C++ Library
# Copyright (C) 2005-2013 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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

#deprecated libtool
#INCLUDE(CreateLibtoolFile)

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_BINARY_DIR}
)

# drumstick-file library

SET(drumstick-file_QTOBJ_SRCS
    include/qsmf.h
    include/qwrk.h
    include/qove.h
)

SET(drumstick-file_HEADERS
    include/macros.h
    include/qsmf.h
    include/qwrk.h
    include/qove.h
)

SET(drumstick-file_SRCS
    file/qsmf.cpp
    file/qwrk.cpp
    file/qove.cpp
)

QT4_WRAP_CPP(drumstick-file_MOC_SRCS ${drumstick-file_QTOBJ_SRCS})

ADD_LIBRARY(drumstick-file
    ${drumstick-file_MOC_SRCS}
    ${drumstick-file_SRCS} 
    ${drumstick-file_HEADERS} 
)

TARGET_LINK_LIBRARIES(drumstick-file
    ${QT_LIBRARIES}
)

IF(STATIC_DRUMSTICK)
    SET_TARGET_PROPERTIES(drumstick-file PROPERTIES
        STATIC_LIB "libdrumstick-file.a"
    )
    IF(NOT WIN32)
        SET_TARGET_PROPERTIES(drumstick-file PROPERTIES
        COMPILE_FLAGS -fPIC
    )
    ENDIF(NOT WIN32)
ELSE(STATIC_DRUMSTICK)
    SET_TARGET_PROPERTIES(drumstick-file PROPERTIES
        VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
        SOVERSION ${VERSION_MAJOR}
#       OUTPUT_NAME drumstick-file-${VERSION_MAJOR}.${VERSION_MINOR}
    )
ENDIF(STATIC_DRUMSTICK)

# drumstick-alsa library
IF(ALSA_FOUND)

    SET(drumstick-alsa_QTOBJ_SRCS
        include/alsaclient.h
        include/alsaevent.h
        include/alsaport.h
        include/alsaqueue.h
        include/alsatimer.h
        include/playthread.h
    )

    SET(drumstick-alsa_HEADERS
        include/alsaclient.h
        include/alsaevent.h
        include/alsaport.h
        include/alsaqueue.h
        include/alsatimer.h
        include/drumstickcommon.h
        include/playthread.h
        include/subscription.h
    )

    SET(drumstick-alsa_SRCS
        alsa/alsaclient.cpp
        alsa/alsaevent.cpp
        alsa/alsaport.cpp
        alsa/alsaqueue.cpp
        alsa/alsatimer.cpp
        alsa/playthread.cpp
        alsa/subscription.cpp
    )

    QT4_WRAP_CPP(drumstick-alsa_MOC_SRCS ${drumstick-alsa_QTOBJ_SRCS})

    ADD_LIBRARY(drumstick-alsa 
        ${drumstick-alsa_MOC_SRCS}
        ${drumstick-alsa_SRCS} 
        ${drumstick-alsa_HEADERS} 
    )

    TARGET_LINK_LIBRARIES(drumstick-alsa
        ${QT_LIBRARIES}
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
#           OUTPUT_NAME drumstick-alsa-${VERSION_MAJOR}.${VERSION_MINOR}
        )
    ENDIF(STATIC_DRUMSTICK)

#deprecated libtool 
#SET_TARGET_PROPERTIES(drumstick-alsa PROPERTIES
#    LT_DEPENDENCY_LIBS "-lasound -lQtCore"
#    LT_VERSION_CURRENT ${VERSION_MAJOR}
#    LT_VERSION_AGE ${VERSION_MINOR}
#    LT_VERSION_REVISION ${VERSION_PATCH}
#)
#CREATE_LIBTOOL_FILE(drumstick-alsa lib${LIB_SUFFIX})

    INSTALL(TARGETS drumstick-alsa 
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib${LIB_SUFFIX}
        LIBRARY DESTINATION lib${LIB_SUFFIX})
    INSTALL(FILES ${drumstick-alsa_HEADERS}
            DESTINATION include/drumstick)
    INSTALL(FILES include/drumstick.h
            DESTINATION include)
            
#drumstick-rt-alsa plugin library

    add_definitions(-DQT_STATICPLUGIN)

    SET(drumstick-rt_QTOBJ_SRCS
        include/rtmidiinput.h
        include/rtmidioutput.h
    )
            
    SET(drumstick-rt-alsa_HEADERS
        rt/alsa/alsamidiinput.h
	rt/alsa/alsamidioutput.h
    )
    
    SET(drumstick-rt-alsa_SRCS
	rt/alsa/alsamidiinput.cpp
	rt/alsa/alsamidioutput.cpp
    )
    
    QT4_WRAP_CPP(drumstick-rt_MOC_SRCS ${drumstick-rt_QTOBJ_SRCS})
    
    ADD_LIBRARY(drumstick-rt-alsa STATIC
        ${drumstick-rt_MOC_SRCS}
        ${drumstick-rt-alsa_SRCS}
        ${drumstick-rt-alsa_HEADERS}
    )
    
    TARGET_LINK_LIBRARIES(drumstick-rt-alsa
        ${QT_LIBRARIES}
        ${ALSA_LIBS}
        drumstick-alsa
    )

    IF(STATIC_DRUMSTICK)
        SET_TARGET_PROPERTIES(drumstick-alsa PROPERTIES
             STATIC_LIB "libdrumstick-rt-alsa.a"
        )
        IF(NOT WIN32)
	    SET_TARGET_PROPERTIES(drumstick-rt-alsa PROPERTIES
	      COMPILE_FLAGS -fPIC
	    )
        ENDIF(NOT WIN32)
    ELSE(STATIC_DRUMSTICK)
        SET_TARGET_PROPERTIES(drumstick-rt-alsa PROPERTIES
            VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
            SOVERSION ${VERSION_MAJOR}
#           OUTPUT_NAME drumstick-alsa-${VERSION_MAJOR}.${VERSION_MINOR}
        )
    ENDIF(STATIC_DRUMSTICK)
    
ENDIF(ALSA_FOUND)

INSTALL(TARGETS drumstick-file
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib${LIB_SUFFIX}
	LIBRARY DESTINATION lib${LIB_SUFFIX})

INSTALL(FILES ${drumstick-file_HEADERS}
        DESTINATION include/drumstick)
