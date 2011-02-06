# MIDI Sequencer C++ Library
# Copyright (C) 2005-2011 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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

SET(vpiano_forms_SRCS
    vpianoabout.ui
    connections.ui
    preferences.ui
    vpiano.ui
)

QT4_WRAP_UI(vpiano_ui_SRCS ${vpiano_forms_SRCS})

SET(vpiano_SRCS
    pianokey.cpp
    pianokey.h
    pianoscene.cpp
    pianoscene.h
    pianokeybd.cpp
    pianokeybd.h
    keylabel.cpp
    keylabel.h
    rawkeybdapp.cpp
    rawkeybdapp.h
    vpianoabout.cpp
    vpianoabout.h
    connectionitem.h
    connections.cpp
    connections.h
    preferences.cpp
    preferences.h
    vpiano.cpp
    vpiano.h
    vpianomain.cpp
)

SET(vpiano_qtobject_SRCS
    pianoscene.h
    pianokeybd.h
    vpianoabout.h
    connections.h
    preferences.h
    vpiano.h
)

QT4_WRAP_CPP(vpiano_moc_SRCS ${vpiano_qtobject_SRCS})

QT4_ADD_RESOURCES(vpiano_SRCS pianokeybd.qrc)

ADD_EXECUTABLE(drumstick-vpiano
    ${vpiano_ui_SRCS}
    ${vpiano_moc_SRCS}
    ${vpiano_SRCS}
)

TARGET_LINK_LIBRARIES(drumstick-vpiano
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    ${QT_X11_X11_LIBRARY}
    drumstick-common
    drumstick-alsa
)

INSTALL(TARGETS drumstick-vpiano
        RUNTIME DESTINATION bin)

INSTALL(FILES drumstick-vpiano.desktop
        DESTINATION share/applications)
