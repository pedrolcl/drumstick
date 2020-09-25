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

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(drumstick-rt-alsa-out_QTOBJ_SRCS
    alsamidioutput.h
)

set(drumstick-rt-alsa-out_SRCS
    alsamidioutput.cpp
)

qt5_wrap_cpp(drumstick-rt-alsa-out_MOC_SRCS ${drumstick-rt-alsa-out_QTOBJ_SRCS}
    OPTIONS -I ${Drumstick_SOURCE_DIR}/library/include
)

if(STATIC_DRUMSTICK)
    add_library(drumstick-rt-alsa-out STATIC
        ${drumstick-rt-alsa-out_MOC_SRCS}
        ${drumstick-rt-alsa-out_SRCS})
    target_compile_definitions(drumstick-rt-alsa-out
        PRIVATE QT_STATICPLUGIN)
    set_target_properties(drumstick-rt-alsa-out PROPERTIES
         STATIC_LIB "libdrumstick-rt-alsa-out")
else()
    add_library(drumstick-rt-alsa-out MODULE
        ${drumstick-rt-alsa-out_MOC_SRCS}
        ${drumstick-rt-alsa-out_SRCS}
    )
    target_compile_definitions(drumstick-rt-alsa-out
        PRIVATE QT_PLUGIN)
endif()

target_link_libraries(drumstick-rt-alsa-out PRIVATE
    Qt5::Core
    Drumstick::ALSA
)

set_target_properties(drumstick-rt-alsa-out PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/${DRUMSTICK_PLUGINS_DIR})

install(TARGETS drumstick-rt-alsa-out
    EXPORT drumstick-rt-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/${DRUMSTICK_PLUGINS_DIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${DRUMSTICK_PLUGINS_DIR})
