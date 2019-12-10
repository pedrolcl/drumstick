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

set(drumstick-rt-oss-in_QTOBJ_SRCS
    ../../include/drumstick/rtmidiinput.h
    ../../include/drumstick/rtmidioutput.h
    ../common/midiparser.h
    ossinput_p.h
    ossinput.h
)

set(drumstick-rt-oss-in_SRCS
    ../common/midiparser.cpp
    ossinput_p.cpp
    ossinput.cpp
)

qt5_wrap_cpp(drumstick-rt-oss-in_MOC_SRCS ${drumstick-rt-oss-in_QTOBJ_SRCS}
    OPTIONS -I ${Drumstick_SOURCE_DIR}/library/include
)

if(STATIC_DRUMSTICK)
    add_library(drumstick-rt-oss-in STATIC
        ${drumstick-rt-oss-in_MOC_SRCS}
        ${drumstick-rt-oss-in_SRCS})
    target_compile_definitions(drumstick-rt-oss-in
        PRIVATE QT_STATICPLUGIN)
    set_target_properties(drumstick-rt-oss-in PROPERTIES
         STATIC_LIB "libdrumstick-rt-oss-in")
else()
    add_library(drumstick-rt-oss-in MODULE
        ${drumstick-rt-oss-in_MOC_SRCS}
        ${drumstick-rt-oss-in_SRCS})
    target_compile_definitions(drumstick-rt-oss-in
        PRIVATE QT_PLUGIN)
endif()

target_include_directories(drumstick-rt-oss-in PRIVATE
    ${Drumstick_SOURCE_DIR}/library/include
    ../common
)

target_link_libraries(drumstick-rt-oss-in PUBLIC
    Qt5::Core
)

set_target_properties(drumstick-rt-oss-in PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/drumstick)

install(TARGETS drumstick-rt-oss-in
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/drumstick
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/drumstick)
