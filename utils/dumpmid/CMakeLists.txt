# MIDI Sequencer C++ Library
# Copyright (C) 2005-2020 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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

set(dumpmid_SRCS
    dumpmid.cpp
    dumpmid.h
)

set(dumpmid_qtobject_SRCS
    dumpmid.h
)

qt5_wrap_cpp(dumpmid_moc_SRCS ${dumpmid_qtobject_SRCS})

add_executable(drumstick-dumpmid
    ${dumpmid_moc_SRCS}
    ${dumpmid_SRCS}
)

target_include_directories(drumstick-dumpmid PRIVATE
    ../common
)

target_link_libraries(drumstick-dumpmid PUBLIC
    ${ALSA_LIBS} 
    Drumstick::ALSA
    Qt5::Core
)

install(TARGETS drumstick-dumpmid
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
