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

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)

find_package(Qt5Widgets REQUIRED)

set(guiplayer_forms_SRCS
    guiplayer.ui
    playerabout.ui
)

set(guiplayer_SRCS
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

set(guiplayer_qtobject_SRCS
    player.h
    playerabout.h
    guiplayer.h
)

qt5_wrap_ui(guiplayer_ui_SRCS ${guiplayer_forms_SRCS})

qt5_wrap_cpp(guiplayer_moc_SRCS ${guiplayer_qtobject_SRCS})

qt5_add_resources(guiplayer_resources guiplayer.qrc)

add_executable(drumstick-guiplayer
    ${guiplayer_resources}
    ${guiplayer_ui_SRCS}
    ${guiplayer_moc_SRCS}
    ${guiplayer_SRCS}
)

target_include_directories(drumstick-guiplayer PRIVATE
    ../common
)

target_link_libraries(drumstick-guiplayer PUBLIC
    ${ALSA_LIBS} 
    Drumstick::File
    Drumstick::ALSA
    Qt5::Widgets
)

install(TARGETS drumstick-guiplayer
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES drumstick-guiplayer.desktop
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
