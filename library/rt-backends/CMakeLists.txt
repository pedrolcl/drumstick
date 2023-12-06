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

set(DRUMSTICK_PLUGINS_DIR "drumstick2")

if(FALSE)
    add_subdirectory(dummy-in)
    add_subdirectory(dummy-out)
endif()

if(BUILD_ALSA AND ALSA_FOUND)
    add_subdirectory(alsa-in)
    add_subdirectory(alsa-out)
endif()

if (HAVE_PULSEAUDIO AND HAVE_SONIVOX)
    add_subdirectory(eassynth)
endif()

if(HAVE_NETWORK)
    add_subdirectory(net-in)
    add_subdirectory(net-out)
endif()

if(UNIX AND NOT APPLE AND NOT EMSCRIPTEN)
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

if(HAVE_FLUIDSYNTH)
    add_subdirectory(fluidsynth)
endif()
