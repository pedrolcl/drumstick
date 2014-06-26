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

SET(drumstick-rt_QTOBJ_SRCS
    ../include/rtmidiinput.h
    ../include/rtmidioutput.h
)

SET(drumstick-rt_SRCS
    backendmanager.cpp
)
    
QT5_WRAP_CPP(drumstick-rt_MOC_SRCS ${drumstick-rt_QTOBJ_SRCS})

ADD_LIBRARY(drumstick-rt
    ${drumstick-rt_MOC_SRCS}
    ${drumstick-rt_SRCS}
)

if(NOT STATIC_DRUMSTICK)
    SET_TARGET_PROPERTIES(drumstick-rt PROPERTIES
        VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
        SOVERSION ${VERSION_MAJOR})
endif()
    

install(TARGETS drumstick-rt
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib${LIB_SUFFIX}
	LIBRARY DESTINATION lib${LIB_SUFFIX})
