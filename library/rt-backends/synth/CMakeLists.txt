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

pkg_check_modules(FLUIDSYNTH REQUIRED fluidsynth>=1.1.1)
include_directories(${FLUIDSYNTH_INCLUDEDIR})
link_directories(${FLUIDSYNTH_LIBDIR})

set(drumstick-rt-synth_QTOBJ_SRCS
    ../../include/rtmidioutput.h
    synthengine.h
    synthoutput.h
)

set(drumstick-rt-synth_SRCS
    synthengine.cpp
    synthoutput.cpp
)

add_definitions(-DQT_PLUGIN)

qt5_wrap_cpp(drumstick-rt-synth_MOC_SRCS ${drumstick-rt-synth_QTOBJ_SRCS})

if(STATIC_DRUMSTICK)
    add_definitions(-DQT_STATICPLUGIN)
    add_library(drumstick-rt-synth STATIC
        ${drumstick-rt-synth_MOC_SRCS}
        ${drumstick-rt-synth_SRCS})
    set_target_properties(drumstick-rt-synth PROPERTIES
         STATIC_LIB "libdrumstick-rt-synth")
else()
    add_library(drumstick-rt-synth MODULE
        ${drumstick-rt-synth_MOC_SRCS}
        ${drumstick-rt-synth_SRCS})
endif()

target_link_libraries(drumstick-rt-synth
    Qt5::Core
    ${FLUIDSYNTH_LIBRARIES})

set_target_properties(drumstick-rt-synth PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/drumstick)

install(TARGETS drumstick-rt-synth
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib${LIB_SUFFIX}/drumstick
        LIBRARY DESTINATION lib${LIB_SUFFIX}/drumstick)
