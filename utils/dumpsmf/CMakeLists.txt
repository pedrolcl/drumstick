#[===========================================================================[
MIDI C++ Library
Copyright (C) 2005-2025 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>

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

set(dumpsmf_SRCS
    dumpsmf.cpp
    dumpsmf.h
)

set(dumpsmf_qtobject_SRCS
    dumpsmf.h
)

if (QT_VERSION VERSION_LESS 5.15.0)
    qt5_wrap_cpp(dumpsmf_moc_SRCS ${dumpsmf_qtobject_SRCS})
else()
    qt_wrap_cpp(dumpsmf_moc_SRCS ${dumpsmf_qtobject_SRCS})
endif()

add_executable(drumstick-dumpsmf
    ${dumpsmf_moc_SRCS}
    ${dumpsmf_SRCS}
)

target_link_libraries(drumstick-dumpsmf PRIVATE
    Drumstick::File
    Qt${QT_VERSION_MAJOR}::Core
)

if(QT_VERSION VERSION_GREATER_EQUAL 6.0.0)
    target_link_libraries(drumstick-dumpsmf PRIVATE Qt6::Core5Compat)
endif()

install(TARGETS drumstick-dumpsmf
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
