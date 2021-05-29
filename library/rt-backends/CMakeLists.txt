# MIDI Sequencer C++ Library
# Copyright (C) 2005-2021 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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

set(DRUMSTICK_PLUGINS_DIR "drumstick2")

if(FALSE)
    add_subdirectory(dummy-in)
    add_subdirectory(dummy-out)
endif()

if(ALSA_FOUND)
    add_subdirectory(alsa-in)
    add_subdirectory(alsa-out)
endif()

if (PULSE_FOUND)
    add_subdirectory(eassynth)
endif()

find_package(Qt${QT_VERSION_MAJOR}Network)
if(Qt${QT_VERSION_MAJOR}Network_FOUND)
    add_subdirectory(net-in)
    add_subdirectory(net-out)
endif()

if(UNIX AND NOT APPLE)
    add_subdirectory(oss-in)
    add_subdirectory(oss-out)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_subdirectory(mac-in)
    add_subdirectory(mac-out)
    add_subdirectory(macsynth)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_subdirectory(win-in)
    add_subdirectory(win-out)
endif()

if (PKG_CONFIG_FOUND)
    pkg_check_modules(FLUIDSYNTH IMPORTED_TARGET fluidsynth>=1.1.1)
    if(FLUIDSYNTH_FOUND)
        add_subdirectory(fluidsynth)
    endif()
endif()
