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

SET ( common_SRCS
    cmdlineargs.cpp
    cmdlineargs.h
)

ADD_LIBRARY( drumstick-common STATIC
    ${common_SRCS}
)

IF(NOT WIN32)
    SET_TARGET_PROPERTIES( drumstick-common 
        PROPERTIES COMPILE_FLAGS -fPIC
	)
ENDIF(NOT WIN32)

TARGET_LINK_LIBRARIES( drumstick-common
    ${QT_LIBRARIES}
)
