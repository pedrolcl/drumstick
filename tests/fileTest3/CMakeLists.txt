#[===========================================================================[
MIDI C++ Library
Copyright (C) 2005-2024 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>

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

add_executable (fileTest3 filetest3.cpp)

target_link_libraries (fileTest3 PRIVATE
    Drumstick::File
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
)

if(QT_VERSION VERSION_GREATER_EQUAL 6.0.0)
    target_link_libraries(fileTest3 PRIVATE Qt6::Core5Compat)
endif()

add_test (fileTest3 ${PROJECT_BINARY_DIR}/bin/fileTest3)
