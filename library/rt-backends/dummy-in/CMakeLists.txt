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

set(drumstick-rt-dummy-in_QTOBJ_SRCS
    ../../include/rtmidiinput.h
    ../../include/rtmidioutput.h
    dummyinput.h
)

set(drumstick-rt-dummy-in_SRCS
    dummyinput.cpp
)

add_definitions(-DQT_PLUGIN)

qt5_wrap_cpp(drumstick-rt-dummy-in_MOC_SRCS ${drumstick-rt-dummy-in_QTOBJ_SRCS})

add_library(drumstick-rt-dummy-in MODULE
    ${drumstick-rt-dummy-in_MOC_SRCS}
    ${drumstick-rt-dummy-in_SRCS}
)

target_link_libraries(drumstick-rt-dummy-in Qt5::Core)

set_target_properties(drumstick-rt-dummy-in PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/drumstick)


install(TARGETS drumstick-rt-dummy-in
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib${LIB_SUFFIX}/drumstick
        LIBRARY DESTINATION lib${LIB_SUFFIX}/drumstick)
