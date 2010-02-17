# MIDI Sequencer C++ Library
# Copyright (C) 2005-2010 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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

# test suite 

# common settings

INCLUDE_DIRECTORIES ( 
    ${CMAKE_CURRENT_BINARY_DIR}
)

LINK_DIRECTORIES (
    ${QT_LIB_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

IF(USE_PCH)
    ADD_PRECOMPILED_HEADER(precompiled_headers stableheaders.h)
ENDIF(USE_PCH)


ADD_SUBDIRECTORY(buildsmf)
ADD_SUBDIRECTORY(dumpsmf)

IF(ALSA_FOUND)
    ADD_SUBDIRECTORY(dumpmid)
    ADD_SUBDIRECTORY(playsmf)
    ADD_SUBDIRECTORY(smfplayer)
    ADD_SUBDIRECTORY(sysinfo)
    ADD_SUBDIRECTORY(testevents)
    ADD_SUBDIRECTORY(timertest)
    ADD_SUBDIRECTORY(vpiano)
    ADD_SUBDIRECTORY(metronome)
ENDIF(ALSA_FOUND)
