# Drumstick PianoKeybd Qt Designer Plugin
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
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5UiPlugin REQUIRED)

set(vpiano-plugin_SRCS
    vpiano-plugin.h
    vpiano-plugin.cpp
    vpiano-plugin.qrc
)

add_library(vpiano-plugin MODULE
    ${vpiano-plugin_SRCS})

target_compile_definitions(vpiano-plugin
    PRIVATE QT_PLUGIN)

target_include_directories(vpiano-plugin PRIVATE
    ${Drumstick_SOURCE_DIR}/library/include
)

target_link_libraries(vpiano-plugin PRIVATE
    Qt5::Widgets
    Qt5::UiPlugin
    Drumstick::Widgets
)

set_target_properties(vpiano-plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/designer)

install(TARGETS vpiano-plugin
    EXPORT vpiano-plugin-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/designer
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/designer)
