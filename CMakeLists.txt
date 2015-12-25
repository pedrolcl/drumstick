# MIDI C++ Library
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

cmake_minimum_required(VERSION 2.8.11)
project(DRUMSTICK)

if(CMAKE_SIZEOF_VOID_P MATCHES "8")
    set(_INIT_LIB_SUFFIX "64")
else()
    set(_INIT_LIB_SUFFIX "")
endif()
set(LIB_SUFFIX ${_INIT_LIB_SUFFIX} CACHE STRING "optional library install directory suffix: 32, 64, none")

set(CMAKE_COLOR_MAKEFILE ON)
#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# Additional CMake modules for 3rd party library checks reside here
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_admin")

# DRUMSTICK version string
set(VERSION_MAJOR "1")
set(VERSION_MINOR "0")
set(VERSION_PATCH "2")
set(VERSION_SUFFIX "")
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_SUFFIX})
add_definitions(-DVERSION=${VERSION})

message(STATUS "drumstick ${VERSION} prefix: ${CMAKE_INSTALL_PREFIX}")

set(_DBUS_INIT OFF)
if(${CMAKE_SYSTEM} MATCHES "Linux")
	set(_DBUS_INIT ON)
endif()

# User options
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
endif()

find_package(PkgConfig REQUIRED)
if(PKG_CONFIG_FOUND)
    message(STATUS "Program pkg-config found (${PKG_CONFIG_EXECUTABLE})")
else()
    message(FATAL_ERROR "Program pkg-config not found")
endif()

if(${CMAKE_SYSTEM} MATCHES "Linux")
    pkg_check_modules(ALSA alsa>=1.0.0)
    if(ALSA_FOUND)
        set(ALSA_LIBS ${ALSA_LIBRARIES})
        list(APPEND ALSA_LIB_DIR ${ALSA_LIBRARY_DIRS} ${ALSA_LIBDIR})
        list(APPEND ALSA_INC_DIR ${ALSA_INCLUDE_DIRS} ${ALSA_INCLUDEDIR})
    else()
        message(STATUS "Warning: ALSA library not found.")
    endif()
endif()

link_directories(${DRUMSTICK_BINARY_DIR}/lib)
include_directories( BEFORE
    ${DRUMSTICK_SOURCE_DIR}/library
    ${DRUMSTICK_SOURCE_DIR}/library/include)

add_subdirectory(library)
add_subdirectory(utils)
add_subdirectory(icons)

if(${CMAKE_SYSTEM} MATCHES "Linux")
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
	DESTINATION lib${LIB_SUFFIX}/pkgconfig )
    # XML mime types
    set( SHARED_MIME_INFO_MINIMUM_VERSION "0.30" )
    set( XDG_MIME_INSTALL_DIR "share/mime/packages" )
    find_package( SharedMimeInfo )
    if( SHARED_MIME_INFO_FOUND )
        install( FILES drumstick.xml DESTINATION ${XDG_MIME_INSTALL_DIR} )
        update_xdg_mimetypes( ${XDG_MIME_INSTALL_DIR} )
    endif()
endif()

configure_file(drumstick-file.pc.in drumstick-file.pc IMMEDIATE @ONLY)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/drumstick-file.pc
    DESTINATION lib${LIB_SUFFIX}/pkgconfig )

configure_file(drumstick-rt.pc.in drumstick-rt.pc IMMEDIATE @ONLY)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/drumstick-rt.pc
    DESTINATION lib${LIB_SUFFIX}/pkgconfig )

# uninstall custom target
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake_admin/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target( uninstall
    "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

if(${CMAKE_SYSTEM} MATCHES "Linux")
    # tarball target
    add_custom_target( tarball
        COMMAND mkdir -p drumstick-${VERSION}
        COMMAND cp -r cmake_admin drumstick-${VERSION}
        COMMAND cp -r library drumstick-${VERSION}
        COMMAND cp -r utils drumstick-${VERSION}
        COMMAND cp -r doc drumstick-${VERSION}
        COMMAND cp -r icons drumstick-${VERSION}
        COMMAND cp CMakeLists.txt AUTHORS COPYING ChangeLog INSTALL NEWS README TODO drumstick*.in drumstick.pro drumstick.xml configure* Doxyfile.in chkcoverage global.pri drumstick-${VERSION}
        COMMAND tar -cj --exclude=.[a-z]* -f drumstick-${VERSION}.tar.bz2 drumstick-${VERSION}
        COMMAND tar -cz --exclude=.[a-z]* -f drumstick-${VERSION}.tar.gz  drumstick-${VERSION}
        COMMAND zip -qr drumstick-${VERSION}.zip drumstick-${VERSION} -x '.[a-z]*'
        COMMAND rm -rf drumstick-${VERSION}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()
