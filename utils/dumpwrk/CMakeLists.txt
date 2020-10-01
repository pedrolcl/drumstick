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

set(dumpwrk_SRCS
    dumpwrk.cpp
    dumpwrk.h
)

set(dumpwrk_qtobject_SRCS
    dumpwrk.h
)

qt5_wrap_cpp(dumpwrk_moc_SRCS ${dumpwrk_qtobject_SRCS})

add_executable(drumstick-dumpwrk
    ${dumpwrk_moc_SRCS}
    ${dumpwrk_SRCS}
)

target_link_libraries(drumstick-dumpwrk PRIVATE
    Drumstick::File
    Qt5::Core
)

install(TARGETS drumstick-dumpwrk
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
