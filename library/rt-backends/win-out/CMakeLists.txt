# MIDI Sequencer C++ Library
# Copyright (C) 2005-2024 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(drumstick-rt-win-out_QTOBJ_SRCS
    winmidioutput.h
)

set(drumstick-rt-win-out_SRCS
    winmidioutput.cpp
)

if (QT_VERSION VERSION_LESS 5.15.0)
    qt5_wrap_cpp(drumstick-rt-win-out_MOC_SRCS ${drumstick-rt-win-out_QTOBJ_SRCS}
        OPTIONS -I ${Drumstick_SOURCE_DIR}/library/include)
else()
    qt_wrap_cpp(drumstick-rt-win-out_MOC_SRCS ${drumstick-rt-win-out_QTOBJ_SRCS}
        OPTIONS -I ${Drumstick_SOURCE_DIR}/library/include)
endif()

if(STATIC_DRUMSTICK)
    add_library(drumstick-rt-win-out STATIC
        ${drumstick-rt-win-out_MOC_SRCS}
        ${drumstick-rt-win-out_SRCS})
    target_compile_definitions(drumstick-rt-win-out
        PRIVATE QT_STATICPLUGIN)
    set_target_properties(drumstick-rt-win-out PROPERTIES
         STATIC_LIB "libdrumstick-rt-win-out")
else()
    add_library(drumstick-rt-win-out MODULE
        ${drumstick-rt-win-out_MOC_SRCS}
        ${drumstick-rt-win-out_SRCS})
    target_compile_definitions(drumstick-rt-win-out
        PRIVATE QT_PLUGIN)
endif()

target_include_directories(drumstick-rt-win-out PRIVATE
    ${Drumstick_SOURCE_DIR}/library/include)

target_link_libraries(drumstick-rt-win-out
    PUBLIC Qt${QT_VERSION_MAJOR}::Core
           Drumstick::RT
    PRIVATE winmm
)

set_target_properties(drumstick-rt-win-out PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/${DRUMSTICK_PLUGINS_DIR})

install(TARGETS drumstick-rt-win-out
        EXPORT drumstick-rt-targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/${DRUMSTICK_PLUGINS_DIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${DRUMSTICK_PLUGINS_DIR})
