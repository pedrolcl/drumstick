# MIDI Sequencer C++ Library
# Copyright (C) 2005-2015 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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

if(STATIC_DRUMSTICK)
  find_package(Qt5Network REQUIRED)
  pkg_check_modules(FLUIDSYNTH REQUIRED fluidsynth>=1.1.1)
  link_directories(${FLUIDSYNTH_LIBDIR})
endif()

set(SRCS
    main.cpp
)

add_executable(drumstick-test-rt
    ${SRCS}
)

target_link_libraries(drumstick-test-rt
    drumstick-common
    drumstick-rt
    Qt5::Core
)

#not an utility, don't install
#install(TARGETS drumstick-test-rt
#        RUNTIME DESTINATION bin)
