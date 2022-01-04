# MIDI Sequencer C++ Library
# Copyright (C) 2005-2022 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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

if (BUILD_FILE)
    add_subdirectory(dumpsmf)
    add_subdirectory(dumprmi)
    add_subdirectory(dumpwrk)
endif()

if(BUILD_ALSA AND ALSA_FOUND)
    add_subdirectory(dumpmid)
    add_subdirectory(sysinfo)
    add_subdirectory(metronome)
    add_subdirectory(drumgrid)
    if(BUILD_FILE)
        add_subdirectory(playsmf)
        add_subdirectory(guiplayer)
    endif()
endif()

if (BUILD_RT AND BUILD_WIDGETS)
    add_subdirectory(vpiano)
endif()
