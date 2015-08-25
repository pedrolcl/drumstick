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
find_package(Qt5Svg REQUIRED)
find_package(Qt5Network REQUIRED)

if(STATIC_DRUMSTICK)
  pkg_check_modules(FLUIDSYNTH REQUIRED fluidsynth>=1.1.1)
  link_directories(${FLUIDSYNTH_LIBDIR})
endif()

set(vpiano_forms_SRCS
    networksettingsdialog.ui
    fluidsettingsdialog.ui
    vpianoabout.ui
    connections.ui
    preferences.ui
    vpiano.ui
)

qt5_wrap_ui(vpiano_ui_SRCS ${vpiano_forms_SRCS})

set(vpiano_SRCS
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
)

set(vpiano_qtobject_SRCS
#    ../../library/include/rtmidiinput.h
#    ../../library/include/rtmidioutput.h
    pianoscene.h
    pianokeybd.h
    vpianoabout.h
    connections.h
    preferences.h
    vpiano.h
    networksettingsdialog.h
    fluidsettingsdialog.h
)

qt5_wrap_cpp(vpiano_moc_SRCS ${vpiano_qtobject_SRCS})

qt5_add_resources(vpiano_SRCS pianokeybd.qrc)

add_executable(drumstick-vpiano
    ${vpiano_ui_SRCS}
    ${vpiano_moc_SRCS}
    ${vpiano_SRCS}
)

target_link_libraries(drumstick-vpiano
    drumstick-common
    drumstick-rt
    Qt5::Widgets
    Qt5::Svg
    Qt5::Network
)

install(TARGETS drumstick-vpiano
        RUNTIME DESTINATION bin)

install(FILES drumstick-vpiano.desktop
        DESTINATION share/applications)
