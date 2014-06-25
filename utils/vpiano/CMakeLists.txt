# MIDI Sequencer C++ Library
# Copyright (C) 2005-2014 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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
include_directories(
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Svg_INCLUDE_DIRS}
)
link_libraries(
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
)
add_definitions(
    ${Qt5Widgets_DEFINITIONS}
    ${Qt5Svg_DEFINITIONS}
)

SET(vpiano_forms_SRCS
    vpianoabout.ui
    connections.ui
    preferences.ui
    vpiano.ui
)

QT5_WRAP_UI(vpiano_ui_SRCS ${vpiano_forms_SRCS})

SET(vpiano_SRCS
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
)

SET(vpiano_qtobject_SRCS
    ../../library/include/rtmidiinput.h
    ../../library/include/rtmidioutput.h
    pianoscene.h
    pianokeybd.h
    vpianoabout.h
    connections.h
    preferences.h
    vpiano.h
)

QT5_WRAP_CPP(vpiano_moc_SRCS ${vpiano_qtobject_SRCS})

QT5_ADD_RESOURCES(vpiano_SRCS pianokeybd.qrc)

ADD_EXECUTABLE(drumstick-vpiano
    ${vpiano_ui_SRCS}
    ${vpiano_moc_SRCS}
    ${vpiano_SRCS}
)

TARGET_LINK_LIBRARIES(drumstick-vpiano
    ${ALSA_LIBS} 
    drumstick-common
    drumstick-alsa
    drumstick-rt
)

INSTALL(TARGETS drumstick-vpiano
        RUNTIME DESTINATION bin)

INSTALL(FILES drumstick-vpiano.desktop
        DESTINATION share/applications)
