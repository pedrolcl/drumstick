# MIDI Sequencer C++ Library
# Copyright (C) 2005-2015 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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

SET(buildsmf_SRCS
    buildsmf.cpp
    buildsmf.h
)

SET(buildsmf_qtobject_SRCS
    buildsmf.h
)

QT5_WRAP_CPP(buildsmf_moc_SRCS ${buildsmf_qtobject_SRCS})

ADD_EXECUTABLE(drumstick-buildsmf 
    ${buildsmf_moc_SRCS}
    ${buildsmf_SRCS}
)

TARGET_LINK_LIBRARIES(drumstick-buildsmf
    drumstick-common
    drumstick-file
    Qt5::Core
)

INSTALL(TARGETS drumstick-buildsmf 
        RUNTIME DESTINATION bin)
