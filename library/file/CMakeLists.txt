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

#INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_INCLUDE_CURRENT_DIR ON)

SET(drumstick-file_QTOBJ_SRCS
    ../include/qsmf.h
    ../include/qwrk.h
    ../include/qove.h
)

SET(drumstick-file_HEADERS
    ../include/macros.h
    ../include/qsmf.h
    ../include/qwrk.h
    ../include/qove.h
)

SET(drumstick-file_SRCS
    qsmf.cpp
    qwrk.cpp
    qove.cpp
)

QT5_WRAP_CPP(drumstick-file_MOC_SRCS ${drumstick-file_QTOBJ_SRCS})

ADD_LIBRARY(drumstick-file
    ${drumstick-file_MOC_SRCS}
    ${drumstick-file_SRCS} 
    ${drumstick-file_HEADERS} 
)

if(STATIC_DRUMSTICK)
    SET_TARGET_PROPERTIES(drumstick-file PROPERTIES
        STATIC_LIB "libdrumstick-file.a"
    )
else()
    SET_TARGET_PROPERTIES(drumstick-file PROPERTIES
        VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
        SOVERSION ${VERSION_MAJOR})
endif()

INSTALL(TARGETS drumstick-file
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib${LIB_SUFFIX}
	LIBRARY DESTINATION lib${LIB_SUFFIX})

INSTALL(FILES ${drumstick-file_HEADERS}
        DESTINATION include/drumstick)
