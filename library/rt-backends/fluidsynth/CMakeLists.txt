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

#pkg_check_modules(FLUIDSYNTH fluidsynth>=1.1.1 fluidsynth<=1.1.11)
link_directories(${FLUIDSYNTH_LIBDIR})

set(drumstick-rt-fluidsynth_QTOBJ_SRCS
    ../../include/drumstick/rtmidioutput.h
    synthengine.h
    synthoutput.h
)

set(drumstick-rt-fluidsynth_SRCS
    synthengine.cpp
    synthoutput.cpp
)

qt5_wrap_cpp(drumstick-rt-fluidsynth_MOC_SRCS ${drumstick-rt-fluidsynth_QTOBJ_SRCS}
    OPTIONS -I ${DRUMSTICK_SOURCE_DIR}/library/include
)

if(STATIC_DRUMSTICK)
    add_library(drumstick-rt-fluidsynth STATIC
        ${drumstick-rt-fluidsynth_MOC_SRCS}
        ${drumstick-rt-fluidsynth_SRCS})
    target_compile_definitions(drumstick-rt-fluidsynth
        PRIVATE QT_STATICPLUGIN)
    set_target_properties(drumstick-rt-fluidsynth PROPERTIES
         STATIC_LIB "libdrumstick-rt-fluidsynth")
else()
    add_library(drumstick-rt-fluidsynth MODULE
        ${drumstick-rt-fluidsynth_MOC_SRCS}
        ${drumstick-rt-fluidsynth_SRCS})
    target_compile_definitions(drumstick-rt-fluidsynth
        PRIVATE QT_PLUGIN)
endif()

target_include_directories(drumstick-rt-fluidsynth PRIVATE
    ${DRUMSTICK_SOURCE_DIR}/library/include
    ${FLUIDSYNTH_INCLUDEDIR}
)

target_link_libraries(drumstick-rt-fluidsynth
    Qt5::Core
    ${FLUIDSYNTH_LIBRARIES})

set_target_properties(drumstick-rt-fluidsynth PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/drumstick)

install(TARGETS drumstick-rt-fluidsynth
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/drumstick
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/drumstick)
