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

set(drumgrid_forms_SRCS
    drumgridabout.ui
    drumgrid.ui
)

set(drumgrid_SRCS
    drumgridabout.cpp
    drumgridabout.h
    drumgrid.cpp
    drumgrid.h
    drumgridmodel.cpp
    drumgridmodel.h
    drumgridmain.cpp
)

set(drumgrid_qtobject_SRCS
    drumgridabout.h
    drumgridmodel.h
    drumgrid.h
)

QT5_WRAP_UI(drumgrid_ui_SRCS ${drumgrid_forms_SRCS})

QT5_WRAP_CPP(drumgrid_moc_SRCS ${drumgrid_qtobject_SRCS})

ADD_EXECUTABLE(drumstick-drumgrid
    ${drumgrid_ui_SRCS}
    ${drumgrid_moc_SRCS}
    ${drumgrid_SRCS}
)

TARGET_LINK_LIBRARIES(drumstick-drumgrid
    ${ALSA_LIBS} 
    drumstick-common
    drumstick-alsa
    Qt5::Widgets
)

INSTALL(TARGETS drumstick-drumgrid 
        RUNTIME DESTINATION bin)

INSTALL(FILES drumstick-drumgrid.desktop
        DESTINATION share/applications)
        
