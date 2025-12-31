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

set(dumprmi_SRCS
    dumprmi.cpp
    dumprmi.h
    main.cpp
    riff.cpp
)

set(dumprmi_qtobject_SRCS
    dumprmi.h
    riff.h
)

if (QT_VERSION VERSION_LESS 5.15.0)
    qt5_wrap_cpp(dumprmi_moc_SRCS ${dumprmi_qtobject_SRCS})
else()
    qt_wrap_cpp(dumprmi_moc_SRCS ${dumprmi_qtobject_SRCS})
endif()

add_executable(drumstick-dumprmi
    ${dumprmi_moc_SRCS}
    ${dumprmi_SRCS}
)

target_link_libraries(drumstick-dumprmi PRIVATE
    Drumstick::File
    Qt${QT_VERSION_MAJOR}::Core
)

if(QT_VERSION VERSION_GREATER_EQUAL 6.0.0)
    target_link_libraries(drumstick-dumprmi PRIVATE Qt6::Core5Compat)
endif()

install(TARGETS drumstick-dumprmi
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
