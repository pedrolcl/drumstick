# MIDI Sequencer C++ Library
# Copyright (C) 2005-2019 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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

SET(playsmf_SRCS
    playsmf.cpp
    playsmf.h
)

SET(playsmf_qtobject_SRCS
    playsmf.h
)

QT5_WRAP_CPP(playsmf_moc_SRCS ${playsmf_qtobject_SRCS})

ADD_EXECUTABLE(drumstick-playsmf
    ${playsmf_moc_SRCS}
    ${playsmf_SRCS}
)

target_include_directories(drumstick-playsmf PRIVATE
    ../common
)

TARGET_LINK_LIBRARIES(drumstick-playsmf
    ${ALSA_LIBS} 
    drumstick-alsa
    drumstick-file
    Qt5::Core
)

# we should install some files...
INSTALL(TARGETS drumstick-playsmf
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
