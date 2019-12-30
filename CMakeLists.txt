# MIDI C++ Library
# Copyright (C) 2005-2019 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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

cmake_minimum_required(VERSION 3.9)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Minimum OS X deployment version")

project(Drumstick
        VERSION 2.0.0
        DESCRIPTION "MIDI C++ Libraries for Qt")

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_admin)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# cmake bug https://gitlab.kitware.com/cmake/cmake/issues/18396 closed in cmake 3.14
if(APPLE AND ${CMAKE_VERSION} VERSION_LESS 3.14)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

include(GNUInstallDirs)

add_definitions(-DVERSION=${PROJECT_VERSION})

message(STATUS "${PROJECT_NAME} v${PROJECT_VERSION} prefix ${CMAKE_INSTALL_PREFIX}")

set(_DBUS_INIT OFF)

option(STATIC_DRUMSTICK "Build static libraries instead of dynamic" OFF)
option(USE_DBUS "Include DBus support (required for RealtimeKit)" ${_DBUS_INIT})

message(STATUS "Build configuration: ${CMAKE_BUILD_TYPE}")

if(STATIC_DRUMSTICK)
    set(BUILD_SHARED_LIBS OFF)
    add_definitions(-DDRUMSTICK_STATIC)
    message(STATUS "Building static libraries")
else()
    set(BUILD_SHARED_LIBS ON)
    message(STATUS "Building dynamic libraries")
endif()

find_package(Qt5Core REQUIRED)
message(STATUS "Found Qt5 version = ${Qt5Core_VERSION_STRING}")

if(USE_DBUS)
    find_package(Qt5DBus REQUIRED)
    add_definitions(-DRTKIT_SUPPORT)
    message(STATUS "Found Qt5DBus version = ${Qt5DBus_VERSION_STRING}")
endif()

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    find_package(PkgConfig REQUIRED)
    if(PKG_CONFIG_FOUND)
        message(STATUS "Program pkg-config ${PKG_CONFIG_VERSION_STRING} found (${PKG_CONFIG_EXECUTABLE})")
    else()
        message(FATAL_ERROR "Program pkg-config not found")
    endif()
endif()

if(${CMAKE_SYSTEM} MATCHES "Linux")
    pkg_check_modules(ALSA alsa>=1.0.0)
endif()

if(PKG_CONFIG_FOUND)
    pkg_check_modules(PULSE libpulse-simple)
    if(NOT PULSE_FOUND)
        message(STATUS "Warning: PulseAudio library not found.")
    endif()
endif()

if(${CMAKE_SYSTEM} MATCHES "Darwin")
    find_package(Qt5Concurrent REQUIRED)
    if(Qt5Concurrent_FOUND)
        message(STATUS "Found Qt5Concurrent version = ${Qt5Concurrent_VERSION_STRING}")
    endif()
endif()

add_subdirectory(library)
add_subdirectory(utils)
add_subdirectory(icons)

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

if(UNIX AND NOT APPLE)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        configure_file(
            "${CMAKE_SOURCE_DIR}/Doxyfile.in"
            "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
            IMMEDIATE @ONLY)
        add_custom_target(doxygen
            ${DOXYGEN} Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    endif()
    add_subdirectory(doc)
    configure_file(drumstick-alsa.pc.in drumstick-alsa.pc IMMEDIATE @ONLY)
    configure_file(drumstick.spec.in drumstick.spec IMMEDIATE @ONLY)
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-alsa.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig )
    # XML mime types
    set( SHARED_MIME_INFO_MINIMUM_VERSION "0.30" )
    set( XDG_MIME_INSTALL_DIR "${CMAKE_INSTALL_DATAROOTDIR}/mime/packages" )
    find_package( SharedMimeInfo )
    if( SHARED_MIME_INFO_FOUND )
        install( FILES drumstick.xml DESTINATION ${XDG_MIME_INSTALL_DIR} )
        update_xdg_mimetypes( ${XDG_MIME_INSTALL_DIR} )
    endif()
endif()

if(PKG_CONFIG_FOUND)
    configure_file(drumstick-file.pc.in drumstick-file.pc IMMEDIATE @ONLY)
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-file.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig )
    configure_file(drumstick-rt.pc.in drumstick-rt.pc IMMEDIATE @ONLY)
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-rt.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig )
endif()

# uninstall custom target
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake_admin/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)
add_custom_target( uninstall
    "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

if(UNIX)
    # tarball target
    add_custom_target( tarball
        COMMAND mkdir -p drumstick-${PROJECT_VERSION}
        COMMAND cp -r cmake_admin drumstick-${PROJECT_VERSION}
        COMMAND cp -r library drumstick-${PROJECT_VERSION}
        COMMAND cp -r utils drumstick-${PROJECT_VERSION}
        COMMAND cp -r doc drumstick-${PROJECT_VERSION}
        COMMAND cp -r icons drumstick-${PROJECT_VERSION}
        COMMAND cp -r tests drumstick-${PROJECT_VERSION}
        COMMAND cp CMakeLists.txt AUTHORS COPYING ChangeLog install NEWS README TODO drumstick*.in drumstick.pro drumstick.xml configure* Doxyfile.in chkcoverage global.pri drumstick-${PROJECT_VERSION}
        COMMAND tar -cj --exclude=.[a-z]* -f drumstick-${PROJECT_VERSION}.tar.bz2 drumstick-${PROJECT_VERSION}
        COMMAND tar -cz --exclude=.[a-z]* -f drumstick-${PROJECT_VERSION}.tar.gz  drumstick-${PROJECT_VERSION}
        COMMAND zip -qr drumstick-${PROJECT_VERSION}.zip drumstick-${PROJECT_VERSION} -x '.[a-z]*'
        COMMAND rm -rf drumstick-${PROJECT_VERSION}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()
