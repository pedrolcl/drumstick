# MIDI Sequencer C++ Library
# Copyright (C) 2005-2018 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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
# along with this program. If not, see <http://www.gnu.org/licenses/>.

SET(dumpsmf_SRCS
    dumpsmf.cpp
    dumpsmf.h
)

SET(dumpsmf_qtobject_SRCS
    dumpsmf.h
)

QT5_WRAP_CPP(dumpsmf_moc_SRCS ${dumpsmf_qtobject_SRCS})

ADD_EXECUTABLE(drumstick-dumpsmf 
    ${dumpsmf_moc_SRCS}
    ${dumpsmf_SRCS}
)

TARGET_LINK_LIBRARIES(drumstick-dumpsmf
    drumstick-file
    Qt5::Core
)

# we should install some files...
INSTALL(TARGETS drumstick-dumpsmf 
        RUNTIME DESTINATION bin)
