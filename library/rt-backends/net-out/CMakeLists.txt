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

find_package(Qt5Network REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(drumstick-rt-net-out_QTOBJ_SRCS
    ../../include/rtmidioutput.h
    netmidioutput.h
)

set(drumstick-rt-net-out_SRCS
    netmidioutput.cpp
)

add_definitions(-DQT_PLUGIN)

qt5_wrap_cpp(drumstick-rt-net-out_MOC_SRCS ${drumstick-rt-net-out_QTOBJ_SRCS})

if(STATIC_DRUMSTICK)
    add_definitions(-DQT_STATICPLUGIN)
    add_library(drumstick-rt-net-out STATIC
        ${drumstick-rt-net-out_MOC_SRCS}
        ${drumstick-rt-net-out_SRCS})
    set_target_properties(drumstick-rt-net-out PROPERTIES
         STATIC_LIB "libdrumstick-rt-net-out")
else()
    add_library(drumstick-rt-net-out MODULE
        ${drumstick-rt-net-out_MOC_SRCS}
        ${drumstick-rt-net-out_SRCS})
endif()

target_link_libraries(drumstick-rt-net-out
    Qt5::Network)

set_target_properties(drumstick-rt-net-out PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/drumstick)

install(TARGETS drumstick-rt-net-out
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib${LIB_SUFFIX}/drumstick
        LIBRARY DESTINATION lib${LIB_SUFFIX}/drumstick)
