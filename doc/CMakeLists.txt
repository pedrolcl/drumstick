# MIDI Sequencer C++ Library 
# Copyright (C) 2008-2010 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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
# You should have received a copy of the GNU General Public License along 
# with this program; If not, see <http://www.gnu.org/licenses/>.

FIND_PROGRAM(XSLTPROC_EXECUTABLE xsltproc)
IF(XSLTPROC_EXECUTABLE)
    MESSAGE(STATUS "XSLTPROC Found: ${XSLTPROC_EXECUTABLE}")
    CONFIGURE_FILE(drumstick-buildsmf.xml.in doc/drumstick-buildsmf.xml IMMEDIATE @ONLY)
    CONFIGURE_FILE(drumstick-drumgrid.xml.in doc/drumstick-drumgrid.xml IMMEDIATE @ONLY)
    CONFIGURE_FILE(drumstick-dumpmid.xml.in doc/drumstick-dumpmid.xml IMMEDIATE @ONLY)
    CONFIGURE_FILE(drumstick-dumpsmf.xml.in doc/drumstick-dumpsmf.xml IMMEDIATE @ONLY)
    CONFIGURE_FILE(drumstick-dumpwrk.xml.in doc/drumstick-dumpwrk.xml IMMEDIATE @ONLY)
    CONFIGURE_FILE(drumstick-metronome.xml.in doc/drumstick-metronome.xml IMMEDIATE @ONLY)
    CONFIGURE_FILE(drumstick-playsmf.xml.in doc/drumstick-playsmf.xml IMMEDIATE @ONLY)
    CONFIGURE_FILE(drumstick-smfplayer.xml.in doc/drumstick-smfplayer.xml IMMEDIATE @ONLY)
    CONFIGURE_FILE(drumstick-sysinfo.xml.in doc/drumstick-sysinfo.xml IMMEDIATE @ONLY)
    CONFIGURE_FILE(drumstick-testevents.xml.in doc/drumstick-testevents.xml IMMEDIATE @ONLY)
    CONFIGURE_FILE(drumstick-timertest.xml.in doc/drumstick-timertest.xml IMMEDIATE @ONLY)
    CONFIGURE_FILE(drumstick-vpiano.xml.in doc/drumstick-vpiano.xml IMMEDIATE @ONLY)
    INCLUDE(CreateManpages)
    CREATE_MANPAGES ( 
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-buildsmf.xml
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-drumgrid.xml
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-dumpmid.xml
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-dumpsmf.xml
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-dumpwrk.xml
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-metronome.xml
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-playsmf.xml
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-smfplayer.xml
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-sysinfo.xml
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-testevents.xml
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-timertest.xml
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-vpiano.xml 
    )
ELSE(XSLTPROC_EXECUTABLE)
    MESSAGE(STATUS "Warning: XSLTPROC NOT Found. Man pages won't be installed")
ENDIF(XSLTPROC_EXECUTABLE)
