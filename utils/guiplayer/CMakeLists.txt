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

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Gui Widgets LinguistTools REQUIRED)

set(guiplayer_forms_SRCS
    guiplayer.ui
    playerabout.ui
)

set(guiplayer_SRCS
    guiplayer.cpp
    guiplayer.h
    iconutils.cpp
    iconutils.h
    player.cpp
    player.h
    playerabout.cpp
    playerabout.h
    playermain.cpp
    song.cpp
    song.h
)

set(guiplayer_qtobject_SRCS
    player.h
    playerabout.h
    guiplayer.h
)

if (QT_VERSION VERSION_LESS 5.15.0)
    qt5_wrap_ui(guiplayer_ui_SRCS ${guiplayer_forms_SRCS})
    qt5_wrap_cpp(guiplayer_moc_SRCS ${guiplayer_qtobject_SRCS})
    qt5_add_resources(guiplayer_resources guiplayer.qrc)
else()
    qt_wrap_ui(guiplayer_ui_SRCS ${guiplayer_forms_SRCS})
    qt_wrap_cpp(guiplayer_moc_SRCS ${guiplayer_qtobject_SRCS})
    qt_add_resources(guiplayer_resources guiplayer.qrc)
endif()

add_executable(drumstick-guiplayer
    ${guiplayer_resources}
    ${guiplayer_ui_SRCS}
    ${guiplayer_moc_SRCS}
    ${guiplayer_SRCS}
)

target_link_libraries(drumstick-guiplayer PRIVATE
    Drumstick::File
    Drumstick::ALSA
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
)

set(TS_FILES
    drumstick-guiplayer_cs.ts
    drumstick-guiplayer_es.ts
    drumstick-guiplayer_ru.ts
)

if (QT_VERSION VERSION_LESS 5.15.0)
    qt5_add_translation(QM_FILES ${TS_FILES})
else()
    qt_add_translation(QM_FILES ${TS_FILES})
endif()

add_custom_target(drumstick-guiplayer-translations ALL DEPENDS ${QM_FILES})

if (UNIX AND NOT APPLE)
    install( FILES ${QM_FILES}
             DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/drumstick )
endif ()

install(TARGETS drumstick-guiplayer
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES drumstick-guiplayer.desktop
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
