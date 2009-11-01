# MIDI Sequencer C++ Library
# Copyright (C) 2005-2009 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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

IF(USE_PCH)
    ADD_PRECOMPILED_HEADER(precompiled_headers stableheaders.h)
ENDIF(USE_PCH)

# sysinfo

SET(sysinfo_SRCS
    sysinfo/sysinfo.cpp
)

ADD_EXECUTABLE(aseqmm-sysinfo
    ${sysinfo_SRCS}
)

IF(USE_PCH)
    ADD_DEPENDENCIES(aseqmm-sysinfo precompiled_headers)
ENDIF(USE_PCH)

TARGET_LINK_LIBRARIES(aseqmm-sysinfo
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    aseqmm
)

# buildsmf 

SET(buildsmf_SRCS
    buildsmf/buildsmf.cpp
    buildsmf/buildsmf.h
)

SET(buildsmf_qtobject_SRCS
    buildsmf/buildsmf.h
)

QT4_WRAP_CPP(buildsmf_moc_SRCS ${buildsmf_qtobject_SRCS})

ADD_EXECUTABLE(aseqmm-buildsmf 
    ${buildsmf_moc_SRCS}
    ${buildsmf_SRCS}
)

IF(USE_PCH)
    ADD_DEPENDENCIES(aseqmm-buildsmf precompiled_headers)
ENDIF(USE_PCH)

TARGET_LINK_LIBRARIES(aseqmm-buildsmf
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

QT4_WRAP_CPP(dumpsmf_moc_SRCS ${dumpsmf_qtobject_SRCS})

ADD_EXECUTABLE(aseqmm-dumpsmf 
    ${dumpsmf_moc_SRCS}
    ${dumpsmf_SRCS}
)

IF(USE_PCH)
    ADD_DEPENDENCIES(aseqmm-dumpsmf precompiled_headers)
ENDIF(USE_PCH)

TARGET_LINK_LIBRARIES(aseqmm-dumpsmf
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

QT4_WRAP_CPP(testevents_moc_SRCS ${testevents_qtobject_SRCS})

ADD_EXECUTABLE(aseqmm-testevents
    ${testevents_moc_SRCS}
    ${testevents_SRCS}
)

IF(USE_PCH)
    ADD_DEPENDENCIES(aseqmm-testevents precompiled_headers)
ENDIF(USE_PCH)

TARGET_LINK_LIBRARIES(aseqmm-testevents
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

QT4_WRAP_CPP(dumpmid_moc_SRCS ${dumpmid_qtobject_SRCS})

ADD_EXECUTABLE(aseqmm-dumpmid
    ${dumpmid_moc_SRCS}
    ${dumpmid_SRCS}
)

IF(USE_PCH)
    ADD_DEPENDENCIES(aseqmm-dumpmid precompiled_headers)
ENDIF(USE_PCH)

TARGET_LINK_LIBRARIES(aseqmm-dumpmid
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    aseqmm
)

# timertest

SET(timertest_SRCS
    timertest/timertest.cpp
    timertest/timertest.h
)

SET(timertest_qtobject_SRCS
    timertest/timertest.h
)

QT4_WRAP_CPP(timertest_moc_SRCS ${timertest_qtobject_SRCS})

ADD_EXECUTABLE(aseqmm-timertest
    ${timertest_SRCS}
    ${timertest_moc_SRCS}
)

IF(USE_PCH)
    ADD_DEPENDENCIES(aseqmm-timertest precompiled_headers)
ENDIF(USE_PCH)

TARGET_LINK_LIBRARIES(aseqmm-timertest
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    aseqmm
)

# playsmf

SET(playsmf_SRCS
    playsmf/playsmf.cpp
    playsmf/playsmf.h
)

SET(playsmf_qtobject_SRCS
    playsmf/playsmf.h
)

QT4_WRAP_CPP(playsmf_moc_SRCS ${playsmf_qtobject_SRCS})

ADD_EXECUTABLE(aseqmm-playsmf
    ${playsmf_moc_SRCS}
    ${playsmf_SRCS}
)

IF(USE_PCH)
    ADD_DEPENDENCIES(aseqmm-playsmf precompiled_headers)
ENDIF(USE_PCH)

TARGET_LINK_LIBRARIES(aseqmm-playsmf
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    aseqmm
)

# playmid

SET(playmid_forms_SRCS
    playmid/smfplayer.ui
)
SET(playmid_SRCS
    playmid/smfplayer.cpp
    playmid/smfplayer.h
    playmid/player.cpp
    playmid/player.h
    playmid/song.cpp
    playmid/song.h
    playmid/main.cpp
)

SET(playmid_qtobject_SRCS
    playmid/player.h
    playmid/smfplayer.h
)

QT4_WRAP_UI(playmid_ui_SRCS ${playmid_forms_SRCS})

QT4_WRAP_CPP(playmid_moc_SRCS ${playmid_qtobject_SRCS})

QT4_ADD_RESOURCES(playmid_resources playmid/smfplayer.qrc)

ADD_EXECUTABLE(aseqmm-smfplayer
    ${playmid_resources}
    ${playmid_ui_SRCS}
    ${playmid_moc_SRCS}
    ${playmid_SRCS}
)

IF(USE_PCH)
    ADD_DEPENDENCIES(aseqmm-smfplayer precompiled_headers)
ENDIF(USE_PCH)

TARGET_LINK_LIBRARIES(aseqmm-smfplayer
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    aseqmm
)

# vpiano

INCLUDE_DIRECTORIES ( 
    ${CMAKE_CURRENT_SOURCE_DIR}/vpiano
)

SET(vpiano_forms_SRCS
    vpiano/about.ui
    vpiano/connections.ui
    vpiano/preferences.ui
    vpiano/vpiano.ui
)

QT4_WRAP_UI(vpiano_ui_SRCS ${vpiano_forms_SRCS})

SET(vpiano_SRCS
    vpiano/pianokey.cpp
    vpiano/pianokey.h
    vpiano/pianoscene.cpp
    vpiano/pianoscene.h
    vpiano/pianokeybd.cpp
    vpiano/pianokeybd.h
    vpiano/keylabel.cpp
    vpiano/keylabel.h
    vpiano/rawkeybdapp.cpp
    vpiano/rawkeybdapp.h
    vpiano/about.cpp
    vpiano/about.h
    vpiano/connectionitem.h
    vpiano/connections.cpp
    vpiano/connections.h
    vpiano/preferences.cpp
    vpiano/preferences.h
    vpiano/vpiano.cpp
    vpiano/vpiano.h
    vpiano/main.cpp
)

SET(vpiano_qtobject_SRCS
    vpiano/pianoscene.h
    vpiano/pianokeybd.h
    vpiano/about.h
    vpiano/connections.h
    vpiano/preferences.h
    vpiano/vpiano.h
)

QT4_WRAP_CPP(vpiano_moc_SRCS ${vpiano_qtobject_SRCS})

QT4_ADD_RESOURCES(vpiano_SRCS vpiano/pianokeybd.qrc)

ADD_EXECUTABLE(aseqmm-vpiano
    ${vpiano_ui_SRCS}
    ${vpiano_moc_SRCS}
    ${vpiano_SRCS}
)

IF(USE_PCH)
    ADD_DEPENDENCIES(aseqmm-vpiano precompiled_headers)
ENDIF(USE_PCH)

TARGET_LINK_LIBRARIES(aseqmm-vpiano
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    aseqmm
)

# we should install some files...
INSTALL(TARGETS aseqmm-buildsmf 
                aseqmm-dumpmid 
                aseqmm-dumpsmf 
                aseqmm-playsmf
                aseqmm-smfplayer
                aseqmm-sysinfo 
                aseqmm-testevents
                aseqmm-timertest 
                aseqmm-vpiano
		RUNTIME DESTINATION bin)
