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

set(playsmf_SRCS
    playsmf.cpp
    playsmf.h
)

set(playsmf_qtobject_SRCS
    playsmf.h
)

qt5_wrap_cpp(playsmf_moc_SRCS ${playsmf_qtobject_SRCS})

add_executable(drumstick-playsmf
    ${playsmf_moc_SRCS}
    ${playsmf_SRCS}
)

target_include_directories(drumstick-playsmf PRIVATE
    ../common
)

target_link_libraries(drumstick-playsmf PUBLIC
    ${ALSA_LIBS} 
    Drumstick::ALSA
    Drumstick::File
    Qt5::Core
)

install(TARGETS drumstick-playsmf
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
