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

add_subdirectory(dumpsmf)
add_subdirectory(dumpwrk)
add_subdirectory(vpiano)

if(ALSA_FOUND)
    add_subdirectory(dumpmid)
    add_subdirectory(playsmf)
    add_subdirectory(guiplayer)
    add_subdirectory(sysinfo)
    add_subdirectory(metronome)
    add_subdirectory(drumgrid)
endif()
