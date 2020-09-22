# MIDI Sequencer C++ Library
# Copyright (C) 2005-2020 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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
# along with this program. If not, see <http://www.gnu.org/licenses/>.

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(drumstick-alsa_QTOBJ_SRCS
    ../include/drumstick/alsaclient.h
    ../include/drumstick/alsaevent.h
    ../include/drumstick/alsaport.h
    ../include/drumstick/alsaqueue.h
    ../include/drumstick/alsatimer.h
    ../include/drumstick/playthread.h
)

set(drumstick-alsa_HEADERS
    ../include/drumstick/alsaclient.h
    ../include/drumstick/alsaevent.h
    ../include/drumstick/alsaport.h
    ../include/drumstick/alsaqueue.h
    ../include/drumstick/alsatimer.h
    ../include/drumstick/playthread.h
    ../include/drumstick/sequencererror.h
    ../include/drumstick/subscription.h
)

set(drumstick-alsa_SRCS
    alsaclient.cpp
    alsaevent.cpp
    alsaport.cpp
    alsaqueue.cpp
    alsatimer.cpp
    playthread.cpp
    sequencererror.cpp
    subscription.cpp
)

qt5_wrap_cpp(drumstick-alsa_MOC_SRCS ${drumstick-alsa_QTOBJ_SRCS})

add_library(drumstick-alsa
    ${drumstick-alsa_MOC_SRCS}
    ${drumstick-alsa_SRCS}
    ${drumstick-alsa_HEADERS}
)

add_library(Drumstick::ALSA ALIAS drumstick-alsa)

target_include_directories(drumstick-alsa PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/library/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(drumstick-alsa PRIVATE
    Qt5::Core
    PkgConfig::ALSA
)

if(USE_DBUS)
    target_link_libraries(drumstick-alsa PRIVATE
        Qt5::DBus
    )
endif()

if(STATIC_DRUMSTICK)
    set_target_properties(drumstick-alsa PROPERTIES
        STATIC_LIB "libdrumstick-alsa")
else()
    set_target_properties(drumstick-alsa PROPERTIES
        VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        EXPORT_NAME ALSA
    )
endif()

install(TARGETS drumstick-alsa
    EXPORT drumstick-alsa-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES ${drumstick-alsa_HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/drumstick)
install(FILES ../include/drumstick.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT drumstick-alsa-targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/drumstick
    NAMESPACE Drumstick::
)

export(EXPORT drumstick-alsa-targets
    NAMESPACE Drumstick::
    FILE ${CMAKE_BINARY_DIR}/drumstick-alsa-targets.cmake
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file( ${CMAKE_BINARY_DIR}/drumstick-alsa-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_file( drumstick-alsa-config.cmake
  ${CMAKE_BINARY_DIR}/drumstick-alsa-config.cmake
  @ONLY
)

install(FILES
    ${CMAKE_BINARY_DIR}/drumstick-alsa-config-version.cmake
    ${CMAKE_BINARY_DIR}/drumstick-alsa-config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/drumstick
)
