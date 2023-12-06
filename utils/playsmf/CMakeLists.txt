#[===========================================================================[
MIDI C++ Library
Copyright (C) 2005-2023 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
#]===========================================================================]

set(playsmf_SRCS
    playsmf.cpp
    playsmf.h
)

set(playsmf_qtobject_SRCS
    playsmf.h
)

if (QT_VERSION VERSION_LESS 5.15.0)
    qt5_wrap_cpp(playsmf_moc_SRCS ${playsmf_qtobject_SRCS})
else()
    qt_wrap_cpp(playsmf_moc_SRCS ${playsmf_qtobject_SRCS})
endif()

add_executable(drumstick-playsmf
    ${playsmf_moc_SRCS}
    ${playsmf_SRCS}
)

target_link_libraries(drumstick-playsmf PRIVATE
    Drumstick::ALSA
    Drumstick::File
    Qt${QT_VERSION_MAJOR}::Core
)

if(QT_VERSION VERSION_GREATER_EQUAL 6.0.0)
    target_link_libraries(drumstick-playsmf PRIVATE Qt6::Core5Compat)
endif()

install(TARGETS drumstick-playsmf
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
