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

find_package(Qt5Widgets REQUIRED)

SET(guiplayer_forms_SRCS
    guiplayer.ui
    playerabout.ui
)

SET(guiplayer_SRCS
    guiplayer.cpp
    guiplayer.h
    player.cpp
    player.h
    song.cpp
    song.h
    playerabout.cpp
    playerabout.h
    playermain.cpp
)

SET(guiplayer_qtobject_SRCS
    player.h
    playerabout.h
    guiplayer.h
)

QT5_WRAP_UI(guiplayer_ui_SRCS ${guiplayer_forms_SRCS})

QT5_WRAP_CPP(guiplayer_moc_SRCS ${guiplayer_qtobject_SRCS})

QT5_ADD_RESOURCES(guiplayer_resources guiplayer.qrc)

ADD_EXECUTABLE(drumstick-guiplayer
    ${guiplayer_resources}
    ${guiplayer_ui_SRCS}
    ${guiplayer_moc_SRCS}
    ${guiplayer_SRCS}
)

TARGET_LINK_LIBRARIES(drumstick-guiplayer
    ${ALSA_LIBS} 
    drumstick-common
    drumstick-file
    drumstick-alsa
    Qt5::Widgets
)

INSTALL(TARGETS drumstick-guiplayer
        RUNTIME DESTINATION bin)

INSTALL(FILES drumstick-guiplayer.desktop
        DESTINATION share/applications)
