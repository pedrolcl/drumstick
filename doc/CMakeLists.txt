# MIDI Sequencer C++ Library 
# Copyright (C) 2008-2021 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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

find_program(XSLTPROC_EXECUTABLE xsltproc)
if(XSLTPROC_EXECUTABLE)
    message(STATUS "XSLTPROC Found: ${XSLTPROC_EXECUTABLE}")
    #unset(RELEASE_DATE)
    #execute_process (
    #    COMMAND bash -c "LANG=C;date +'%B %d, %Y'"
    #    OUTPUT_VARIABLE RELEASE_DATE
    #    OUTPUT_STRIP_TRAILING_WHITESPACE
    #)
    set(RELEASE_DATE "July 29, 2021")
    configure_file(drumstick-devel.doc.txt.in
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-devel.doc.txt IMMEDIATE @ONLY)
    configure_file(drumstick-drumgrid.xml.in 
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-drumgrid.xml IMMEDIATE @ONLY)
    configure_file(drumstick-dumpmid.xml.in 
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-dumpmid.xml IMMEDIATE @ONLY)
    configure_file(drumstick-dumpsmf.xml.in 
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-dumpsmf.xml IMMEDIATE @ONLY)
    configure_file(drumstick-dumpwrk.xml.in 
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-dumpwrk.xml IMMEDIATE @ONLY)
    configure_file(drumstick-metronome.xml.in 
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-metronome.xml IMMEDIATE @ONLY)
    configure_file(drumstick-playsmf.xml.in 
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-playsmf.xml IMMEDIATE @ONLY)
    configure_file(drumstick-guiplayer.xml.in 
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-guiplayer.xml IMMEDIATE @ONLY)
    configure_file(drumstick-sysinfo.xml.in 
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-sysinfo.xml IMMEDIATE @ONLY)
    configure_file(drumstick-vpiano.xml.in 
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-vpiano.xml IMMEDIATE @ONLY)
    include(CreateManpages)
    create_manpages ( 
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-drumgrid.xml
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-dumpmid.xml
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-dumpsmf.xml
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-dumpwrk.xml
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-metronome.xml
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-playsmf.xml
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-guiplayer.xml
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-sysinfo.xml
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-vpiano.xml 
    )
else()
    message(STATUS "Warning: XSLTPROC NOT Found. Man pages won't be installed")
endif()
