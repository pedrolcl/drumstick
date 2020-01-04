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
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)

set(vpiano_SRCS
    networksettingsdialog.ui
    fluidsettingsdialog.ui
    macsynthsettingsdialog.ui
    sonivoxsettingsdialog.ui
    vpianoabout.ui
    connections.ui
    preferences.ui
    vpiano.ui
    pianokey.cpp
    pianokey.h
    pianoscene.cpp
    pianoscene.h
    pianokeybd.cpp
    pianokeybd.h
    keylabel.cpp
    keylabel.h
    vpianoabout.cpp
    vpianoabout.h
    connections.cpp
    connections.h
    preferences.cpp
    preferences.h
    vpiano.cpp
    vpiano.h
    vpianomain.cpp
    networksettingsdialog.cpp
    fluidsettingsdialog.cpp
    macsynthsettingsdialog.cpp
    sonivoxsettingsdialog.cpp
    pianokeybd.qrc
)

add_executable(drumstick-vpiano
    ${vpiano_SRCS}
)

target_include_directories(drumstick-vpiano
    PRIVATE ../common
)

target_link_libraries(drumstick-vpiano PUBLIC
    Drumstick::RT
    Qt5::Widgets
    Qt5::Network
)

install(TARGETS drumstick-vpiano
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES drumstick-vpiano.desktop
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
