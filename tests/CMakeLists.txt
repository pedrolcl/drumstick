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

find_package(Qt${QT_VERSION_MAJOR}Test REQUIRED)
set (CMAKE_AUTOMOC ON)
enable_testing()

if (BUILD_ALSA AND ALSA_FOUND)
    add_subdirectory(alsaTest1)
    add_subdirectory(alsaTest2)
endif()

if (BUILD_FILE)
    add_subdirectory(fileTest1)
    add_subdirectory(fileTest2)
    add_subdirectory(fileTest3)
endif()

if (BUILD_RT)
    add_subdirectory(rtTest)
    if(BUILD_WIDGETS)
        add_subdirectory(widgetsTest)
    endif()
endif()
