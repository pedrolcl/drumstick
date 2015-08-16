# MIDI Sequencer C++ Library 
# Copyright (C) 2008-2015 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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
    CONFIGURE_FILE(drumstick-buildsmf.xml.in 
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-buildsmf.xml IMMEDIATE @ONLY)
    CONFIGURE_FILE(drumstick-drumgrid.xml.in 
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-drumgrid.xml IMMEDIATE @ONLY)
    CONFIGURE_FILE(drumstick-dumpmid.xml.in 
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-dumpmid.xml IMMEDIATE @ONLY)
    CONFIGURE_FILE(drumstick-dumpove.xml.in 
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-dumpove.xml IMMEDIATE @ONLY)
    CONFIGURE_FILE(drumstick-dumpsmf.xml.in 
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-dumpsmf.xml IMMEDIATE @ONLY)
    CONFIGURE_FILE(drumstick-dumpwrk.xml.in 
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-dumpwrk.xml IMMEDIATE @ONLY)
    CONFIGURE_FILE(drumstick-metronome.xml.in 
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-metronome.xml IMMEDIATE @ONLY)
    CONFIGURE_FILE(drumstick-playsmf.xml.in 
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-playsmf.xml IMMEDIATE @ONLY)
    CONFIGURE_FILE(drumstick-guiplayer.xml.in 
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-guiplayer.xml IMMEDIATE @ONLY)
    CONFIGURE_FILE(drumstick-sysinfo.xml.in 
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-sysinfo.xml IMMEDIATE @ONLY)
    CONFIGURE_FILE(drumstick-vpiano.xml.in 
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-vpiano.xml IMMEDIATE @ONLY)
    INCLUDE(CreateManpages)
    CREATE_MANPAGES ( 
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-buildsmf.xml
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-drumgrid.xml
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-dumpmid.xml
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-dumpove.xml
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-dumpsmf.xml
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-dumpwrk.xml
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-metronome.xml
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-playsmf.xml
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-guiplayer.xml
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-sysinfo.xml
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-vpiano.xml 
    )
ELSE(XSLTPROC_EXECUTABLE)
    MESSAGE(STATUS "Warning: XSLTPROC NOT Found. Man pages won't be installed")
ENDIF(XSLTPROC_EXECUTABLE)
