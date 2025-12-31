# MIDI Sequencer C++ Library
# Copyright (C) 2005-2025 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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

find_package(Qt${QT_VERSION_MAJOR}Widgets REQUIRED)

set(drumstick-rt-dummy-in_QTOBJ_SRCS
    dummyinput.h
)

set(drumstick-rt-dummy-in_SRCS
    dummyinput.cpp
)

if (QT_VERSION VERSION_LESS 5.15.0)
    qt5_wrap_cpp(drumstick-rt-dummy-in_MOC_SRCS ${drumstick-rt-dummy-in_QTOBJ_SRCS}
        OPTIONS -I ${Drumstick_SOURCE_DIR}/library/include)
else()
    qt_wrap_cpp(drumstick-rt-dummy-in_MOC_SRCS ${drumstick-rt-dummy-in_QTOBJ_SRCS}
        OPTIONS -I ${Drumstick_SOURCE_DIR}/library/include)
endif()

if(STATIC_DRUMSTICK)
    add_library(drumstick-rt-dummy-in STATIC
        ${drumstick-rt-dummy-in_MOC_SRCS}
        ${drumstick-rt-dummy-in_SRCS}
    )
    target_compile_definitions(drumstick-rt-dummy-in
        PRIVATE QT_STATICPLUGIN)
    set_target_properties(drumstick-rt-dummy-in PROPERTIES
        STATIC_LIB "libdrumstick-rt-dummy-in")
else()
    add_library(drumstick-rt-dummy-in MODULE
        ${drumstick-rt-dummy-in_MOC_SRCS}
        ${drumstick-rt-dummy-in_SRCS}
    )
    target_compile_definitions(drumstick-rt-dummy-in
        PRIVATE QT_PLUGIN)
endif()

target_link_libraries(drumstick-rt-dummy-in PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Drumstick::RT
)

target_include_directories(drumstick-rt-dummy-in PRIVATE
    ${Drumstick_SOURCE_DIR}/library/include
)

set_target_properties(drumstick-rt-dummy-in PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/${DRUMSTICK_PLUGINS_DIR})

install(TARGETS drumstick-rt-dummy-in
    EXPORT drumstick-rt-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/${DRUMSTICK_PLUGINS_DIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${DRUMSTICK_PLUGINS_DIR})
