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

SET(timertest_SRCS
    timertest.cpp
    timertest.h
)

SET(timertest_qtobject_SRCS
    timertest.h
)

QT5_WRAP_CPP(timertest_moc_SRCS ${timertest_qtobject_SRCS})

ADD_EXECUTABLE(drumstick-timertest
    ${timertest_SRCS}
    ${timertest_moc_SRCS}
)

TARGET_LINK_LIBRARIES(drumstick-timertest
    ${ALSA_LIBS} 
    drumstick-common
    drumstick-alsa
    Qt5::Core
)

# not an utility
#INSTALL(TARGETS drumstick-timertest
#        RUNTIME DESTINATION bin)
