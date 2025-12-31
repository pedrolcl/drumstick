#[===========================================================================[
MIDI C++ Library
Copyright (C) 2005-2025 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
#]===========================================================================]

find_package(Qt${QT_VERSION_MAJOR}Network REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(drumstick-rt-net-in_QTOBJ_SRCS
    ../common/midiparser.h
    netmidiinput_p.h
    netmidiinput.h
)

set(drumstick-rt-net-in_SRCS
    ../common/midiparser.cpp
    netmidiinput_p.cpp
    netmidiinput.cpp
)

if (QT_VERSION VERSION_LESS 5.15.0)
    qt5_wrap_cpp(drumstick-rt-net-in_MOC_SRCS ${drumstick-rt-net-in_QTOBJ_SRCS}
        OPTIONS -I ${Drumstick_SOURCE_DIR}/library/include)
else()
    qt_wrap_cpp(drumstick-rt-net-in_MOC_SRCS ${drumstick-rt-net-in_QTOBJ_SRCS}
        OPTIONS -I ${Drumstick_SOURCE_DIR}/library/include)
endif()

if(STATIC_DRUMSTICK)
    add_library(drumstick-rt-net-in STATIC
        ${drumstick-rt-net-in_MOC_SRCS}
        ${drumstick-rt-net-in_SRCS})
    target_compile_definitions(drumstick-rt-net-in
        PRIVATE QT_STATICPLUGIN)
    set_target_properties(drumstick-rt-net-in PROPERTIES
         STATIC_LIB "libdrumstick-rt-net-in")
else()
    add_library(drumstick-rt-net-in MODULE
        ${drumstick-rt-net-in_MOC_SRCS}
        ${drumstick-rt-net-in_SRCS})
    target_compile_definitions(drumstick-rt-net-in
        PRIVATE QT_PLUGIN)
endif()

target_include_directories(drumstick-rt-net-in PRIVATE
    ${Drumstick_SOURCE_DIR}/library/include
    ../common )

target_link_libraries(drumstick-rt-net-in PRIVATE
    Qt${QT_VERSION_MAJOR}::Network
    Drumstick::RT
)

set_target_properties(drumstick-rt-net-in PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/${DRUMSTICK_PLUGINS_DIR})

install(TARGETS drumstick-rt-net-in
    EXPORT drumstick-rt-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/${DRUMSTICK_PLUGINS_DIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${DRUMSTICK_PLUGINS_DIR})
