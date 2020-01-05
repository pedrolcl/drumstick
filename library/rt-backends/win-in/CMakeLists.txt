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

set(drumstick-rt-win-in_QTOBJ_SRCS
    winmidiinput.h
)

set(drumstick-rt-win-in_SRCS
    winmidiinput.cpp
)

qt5_wrap_cpp(drumstick-rt-win-in_MOC_SRCS ${drumstick-rt-win-in_QTOBJ_SRCS}
    OPTIONS -I ${Drumstick_SOURCE_DIR}/library/include
)

if(STATIC_DRUMSTICK)
    add_library(drumstick-rt-win-in STATIC
        ${drumstick-rt-win-in_MOC_SRCS}
        ${drumstick-rt-win-in_SRCS})
    target_compile_definitions(drumstick-rt-win-in
        PRIVATE QT_STATICPLUGIN)
    set_target_properties(drumstick-rt-win-in PROPERTIES
         STATIC_LIB "libdrumstick-rt-win-in")
else()
    add_library(drumstick-rt-win-in MODULE
        ${drumstick-rt-win-in_MOC_SRCS}
        ${drumstick-rt-win-in_SRCS})
    target_compile_definitions(drumstick-rt-win-in
        PRIVATE QT_PLUGIN)
endif()

target_link_libraries(drumstick-rt-win-in
    PUBLIC Qt5::Core
           Drumstick::RT
    PRIVATE winmm
)

target_include_directories(drumstick-rt-win-in PRIVATE
    ${Drumstick_SOURCE_DIR}/library/include)

set_target_properties(drumstick-rt-win-in PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/drumstick)

install(TARGETS drumstick-rt-win-in
        EXPORT drumstick-rt-targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/drumstick
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/drumstick)
