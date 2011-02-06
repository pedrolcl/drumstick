# MIDI Sequencer C++ Library
# Copyright (C) 2005-2011 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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

# common settings

INCLUDE_DIRECTORIES ( 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_BINARY_DIR}/common
)

FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/common/cmdversion.h
     "const QString PGM_VERSION(\"${VERSION}\");\n")

ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(buildsmf)
ADD_SUBDIRECTORY(dumpsmf)
ADD_SUBDIRECTORY(dumpwrk)
ADD_SUBDIRECTORY(dumpove)

IF(ALSA_FOUND)
    ADD_SUBDIRECTORY(dumpmid)
    ADD_SUBDIRECTORY(playsmf)
    ADD_SUBDIRECTORY(guiplayer)
    ADD_SUBDIRECTORY(sysinfo)
    ADD_SUBDIRECTORY(testevents)
    ADD_SUBDIRECTORY(timertest)
    ADD_SUBDIRECTORY(vpiano)
    ADD_SUBDIRECTORY(metronome)
    ADD_SUBDIRECTORY(drumgrid)
ENDIF(ALSA_FOUND)
