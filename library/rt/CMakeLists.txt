# MIDI Sequencer C++ Library
# Copyright (C) 2005-2014 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(drumstick-rt_HEADERS
    ../include/rtmidiinput.h
    ../include/rtmidioutput.h
)

set(drumstick-rt_SRCS
    backendmanager.cpp
)
    
qt5_wrap_cpp(drumstick-rt_MOC_SRCS ${drumstick-rt_HEADERS})

add_compiler_export_flags()

add_library(drumstick-rt
    ${drumstick-rt_MOC_SRCS}
    ${drumstick-rt_SRCS}
    ${drumstick-rt_HEADERS}
)

target_link_libraries(drumstick-rt Qt5::Core)

if(STATIC_DRUMSTICK)
    set_target_properties(drumstick-rt PROPERTIES
        STATIC_LIB "libdrumstick-rt.a")
else()
    set_target_properties(drumstick-rt PROPERTIES
        VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
        SOVERSION ${VERSION_MAJOR})
endif()

install(TARGETS drumstick-rt
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib${LIB_SUFFIX}
	LIBRARY DESTINATION lib${LIB_SUFFIX})

install(FILES ${drumstick-rt_HEADERS}
        DESTINATION include/drumstick)
