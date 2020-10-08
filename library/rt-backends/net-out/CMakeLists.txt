# MIDI Sequencer C++ Library
# Copyright (C) 2005-2020 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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

find_package(Qt5Network REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(drumstick-rt-net-out_QTOBJ_SRCS
    netmidioutput.h
)

set(drumstick-rt-net-out_SRCS
    netmidioutput.cpp
)

qt5_wrap_cpp(drumstick-rt-net-out_MOC_SRCS ${drumstick-rt-net-out_QTOBJ_SRCS}
    OPTIONS -I ${Drumstick_SOURCE_DIR}/library/include
)

if(STATIC_DRUMSTICK)
    add_library(drumstick-rt-net-out STATIC
        ${drumstick-rt-net-out_MOC_SRCS}
        ${drumstick-rt-net-out_SRCS})
    target_compile_definitions(drumstick-rt-net-out
        PRIVATE QT_STATICPLUGIN)
    set_target_properties(drumstick-rt-net-out PROPERTIES
        STATIC_LIB "libdrumstick-rt-net-out")
else()
    add_library(drumstick-rt-net-out MODULE
        ${drumstick-rt-net-out_MOC_SRCS}
        ${drumstick-rt-net-out_SRCS})
    target_compile_definitions(drumstick-rt-net-out
        PRIVATE QT_PLUGIN)
endif()

target_include_directories(drumstick-rt-net-out PRIVATE
    ${Drumstick_SOURCE_DIR}/library/include
)

target_link_libraries(drumstick-rt-net-out PRIVATE
    Qt5::Network
    Drumstick::RT
)

set_target_properties(drumstick-rt-net-out PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/${DRUMSTICK_PLUGINS_DIR})

install(TARGETS drumstick-rt-net-out
    EXPORT drumstick-rt-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/${DRUMSTICK_PLUGINS_DIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${DRUMSTICK_PLUGINS_DIR})
