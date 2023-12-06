#[===========================================================================[
MIDI C++ Library
Copyright (C) 2005-2023 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>

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

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt${QT_VERSION_MAJOR}Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR}UiPlugin REQUIRED)

set(SOURCES
    vpiano-plugin.h
    vpiano-plugin.cpp
    vpiano-plugin.qrc)

add_library(drumstick-vpiano-plugin MODULE ${SOURCES})

target_compile_definitions(drumstick-vpiano-plugin PRIVATE QT_PLUGIN)

target_include_directories(drumstick-vpiano-plugin PRIVATE
    ${Drumstick_SOURCE_DIR}/library/include
)

target_link_libraries(drumstick-vpiano-plugin PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::UiPlugin
    Drumstick::Widgets
)

set_target_properties(drumstick-vpiano-plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/designer)

get_target_property(QT_QMAKE_EXECUTABLE Qt${QT_VERSION_MAJOR}::qmake LOCATION)
execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_PLUGINS
  OUTPUT_VARIABLE QT_INSTALL_PLUGINS OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_HOST_PREFIX
  OUTPUT_VARIABLE QT_HOST_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REPLACE "${QT_HOST_PREFIX}/" "" _INSTALL_PLUGINS  "${QT_INSTALL_PLUGINS}")

install(TARGETS drumstick-vpiano-plugin
    EXPORT drumstick-vpiano-targets
    RUNTIME DESTINATION ${_INSTALL_PLUGINS}/designer
    ARCHIVE DESTINATION ${_INSTALL_PLUGINS}/designer
    LIBRARY DESTINATION ${_INSTALL_PLUGINS}/designer)
