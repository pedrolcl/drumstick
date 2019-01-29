# MIDI Sequencer C++ Library
# Copyright (C) 2005-2019 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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

#include_directories(../common)

set(drumstick-rt-macsynth_QTOBJ_SRCS
    ../../include/drumstick/rtmidioutput.h
    macsynth.h
)

set(drumstick-rt-macsynth_SRCS
    macsynth.cpp
)

qt5_wrap_cpp(drumstick-rt-macsynth_MOC_SRCS ${drumstick-rt-macsynth_QTOBJ_SRCS}
    OPTIONS -I ${DRUMSTICK_SOURCE_DIR}/library/include
)

if(STATIC_DRUMSTICK)
    add_library(drumstick-rt-macsynth STATIC
        ${drumstick-rt-macsynth_MOC_SRCS}
        ${drumstick-rt-macsynth_SRCS})
    target_compile_definitions(drumstick-rt-macsynth
        PRIVATE QT_STATICPLUGIN)
    set_target_properties(drumstick-rt-macsynth PROPERTIES
         STATIC_LIB "libdrumstick-rt-macsynth")
else()
    add_library(drumstick-rt-macsynth MODULE
        ${drumstick-rt-macsynth_MOC_SRCS}
        ${drumstick-rt-macsynth_SRCS})
    target_compile_definitions(drumstick-rt-macsynth
        PRIVATE QT_PLUGIN)
endif()

target_link_libraries(drumstick-rt-macsynth
    Qt5::Core
    "-framework CoreMidi -framework CoreFoundation -framework CoreServices"
    "-framework CoreAudio -framework AudioToolbox -framework AudioUnit"
)

target_include_directories(drumstick-rt-macsynth PRIVATE
    ${DRUMSTICK_SOURCE_DIR}/library/include
    ../common
)

set_target_properties(drumstick-rt-macsynth PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/drumstick)

install(TARGETS drumstick-rt-macsynth
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/drumstick
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/drumstick)
