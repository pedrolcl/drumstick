# MIDI Sequencer C++ Library
# Copyright (C) 2005-2021 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
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
find_package(Qt5LinguistTools REQUIRED)

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

qt5_wrap_ui(drumgrid_ui_SRCS ${drumgrid_forms_SRCS})

qt5_wrap_cpp(drumgrid_moc_SRCS ${drumgrid_qtobject_SRCS})

add_executable(drumstick-drumgrid
    ${drumgrid_ui_SRCS}
    ${drumgrid_moc_SRCS}
    ${drumgrid_SRCS}
)

target_link_libraries(drumstick-drumgrid PRIVATE
    Drumstick::ALSA
    Qt5::Widgets
)

set(TS_FILES
    drumstick-drumgrid_en.ts
    drumstick-drumgrid_es.ts
)

qt5_add_translation(QM_FILES ${TS_FILES})
add_custom_target(drumstick-drumgrid-translations ALL DEPENDS ${QM_FILES})

if (UNIX AND NOT APPLE)
    install( FILES ${QM_FILES}
             DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/drumstick )
endif ()

install(TARGETS drumstick-drumgrid
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES drumstick-drumgrid.desktop
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")

