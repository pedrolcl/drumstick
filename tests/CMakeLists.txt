# MIDI Sequencer C++ Library
# Copyright (C) 2005-2008 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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

# tests executables

# common settings

INCLUDE_DIRECTORIES ( 
    ${CMAKE_CURRENT_BINARY_DIR}
)

LINK_DIRECTORIES (
    ${QT_LIB_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# buildsmf 

SET(buildsmf_SRCS
    buildsmf/buildsmf.cpp
    buildsmf/buildsmf.h
)

SET(buildsmf_qtobject_SRCS
    buildsmf/buildsmf.h
)

QT_WRAP_CPP(buildsmf buildsmf_moc_SRCS ${buildsmf_qtobject_SRCS})

ADD_EXECUTABLE(buildsmf 
    ${buildsmf_moc_SRCS}
    ${buildsmf_SRCS}
)

TARGET_LINK_LIBRARIES(buildsmf
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    aseqmm
)

# dumpsmf

SET(dumpsmf_SRCS
    dumpsmf/dumpsmf.cpp
    dumpsmf/dumpsmf.h
)

SET(dumpsmf_qtobject_SRCS
    dumpsmf/dumpsmf.h
)

QT_WRAP_CPP(dumpsmf dumpsmf_moc_SRCS ${dumpsmf_qtobject_SRCS})

ADD_EXECUTABLE(dumpsmf 
    ${dumpsmf_moc_SRCS}
    ${dumpsmf_SRCS}
)

TARGET_LINK_LIBRARIES(dumpsmf
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    aseqmm
)

# testevents

SET(testevents_SRCS
    testevents/testevents.cpp
    testevents/testevents.h
)

SET(testevents_qtobject_SRCS
    testevents/testevents.h
)

QT_WRAP_CPP(testevents testevents_moc_SRCS ${testevents_qtobject_SRCS})

ADD_EXECUTABLE(testevents
    ${testevents_moc_SRCS}
    ${testevents_SRCS}
)

TARGET_LINK_LIBRARIES(testevents
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    aseqmm
)

# dumpmid

SET(dumpmid_SRCS
    dumpmid/dumpmid.cpp
    dumpmid/dumpmid.h
)

SET(dumpmid_qtobject_SRCS
    dumpmid/dumpmid.h
)

QT_WRAP_CPP(dumpmid dumpmid_moc_SRCS ${dumpmid_qtobject_SRCS})

ADD_EXECUTABLE(dumpmid
    ${dumpmid_moc_SRCS}
    ${dumpmid_SRCS}
)

TARGET_LINK_LIBRARIES(dumpmid
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    aseqmm
)

# we should install some files...
INSTALL(TARGETS buildsmf dumpsmf testevents dumpmid
		RUNTIME DESTINATION bin
		ARCHIVE DESTINATION lib
		LIBRARY DESTINATION lib)
