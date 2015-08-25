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

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(drumstick-rt-win-in_QTOBJ_SRCS
    ../../include/rtmidiinput.h
    ../../include/rtmidioutput.h
    winmidiinput.h
)

set(drumstick-rt-win-in_SRCS
    winmidiinput.cpp
)

add_definitions(-DQT_PLUGIN)

qt5_wrap_cpp(drumstick-rt-win-in_MOC_SRCS ${drumstick-rt-win-in_QTOBJ_SRCS})

if(STATIC_DRUMSTICK)
    add_definitions(-DQT_STATICPLUGIN)
    add_library(drumstick-rt-win-in STATIC
        ${drumstick-rt-win-in_MOC_SRCS}
        ${drumstick-rt-win-in_SRCS})
    set_target_properties(drumstick-rt-win-in PROPERTIES
         STATIC_LIB "libdrumstick-rt-win-in")
else()
    add_library(drumstick-rt-win-in MODULE
        ${drumstick-rt-win-in_MOC_SRCS}
        ${drumstick-rt-win-in_SRCS})
endif()

target_link_libraries(drumstick-rt-win-in
    Qt5::Core
    winmm)

set_target_properties(drumstick-rt-win-in PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/drumstick)

install(TARGETS drumstick-rt-win-in
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib${LIB_SUFFIX}/drumstick
        LIBRARY DESTINATION lib${LIB_SUFFIX}/drumstick)
