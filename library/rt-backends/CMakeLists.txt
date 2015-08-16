# MIDI Sequencer C++ Library
# Copyright (C) 2005-2015 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA 02110-1301, USA

#add_subdirectory(dummy-in)
#add_subdirectory(dummy-out)

if(ALSA_FOUND)
    add_subdirectory(alsa-in)
    add_subdirectory(alsa-out)
endif()

find_package(Qt5Network)
if(Qt5Network_FOUND)
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
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_subdirectory(win-in)
    add_subdirectory(win-out)
endif()

pkg_check_modules(FLUIDSYNTH fluidsynth>=1.1.1)
if(FLUIDSYNTH_FOUND)
    add_subdirectory(synth)
endif()
