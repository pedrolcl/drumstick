# MIDI Sequencer C++ Library
# Copyright (C) 2005-2010 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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

# The name of our project is "DRUMSTICK". CMakeLists files in this project can
# refer to the root source directory of the project as ${DRUMSTICK_SOURCE_DIR} 
# and to the root binary directory of the project as ${DRUMSTICK_BINARY_DIR}
PROJECT(DRUMSTICK)

# CMake minimum version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

#SET(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH
#	"Install path prefix, prepended onto install directories." FORCE)

IF(CMAKE_SIZEOF_VOID_P MATCHES "8")
    SET(LIB "lib64" CACHE STRING "library install directory: lib, lib32, lib64")
ELSE(CMAKE_SIZEOF_VOID_P MATCHES "8")
    SET(LIB "lib" CACHE STRING "library install directory: lib, lib32, lib64")
ENDIF(CMAKE_SIZEOF_VOID_P MATCHES "8")

# Seems that relative paths break the link scripts. Don't set it ON
SET(CMAKE_USE_RELATIVE_PATHS OFF)

# Assorted settings
SET(CMAKE_COLOR_MAKEFILE ON)
SET(CMAKE_INCLUDE_CURRENT_DIR TRUE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
SET(CMAKE_SKIP_RULE_DEPENDENCY FALSE)

# Additional CMake modules for 3rd party library checks reside here
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_admin")

# DRUMSTICK version string
SET (VERSION_MAJOR "0")
SET (VERSION_MINOR "3")
SET (VERSION_PATCH "0")
SET (VERSION_SUFFIX "")
SET (VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_SUFFIX})
ADD_DEFINITIONS(-D'VERSION="${VERSION}"')

MESSAGE(STATUS "drumstick ${VERSION} prefix: ${CMAKE_INSTALL_PREFIX}")

# User options
OPTION(STATIC_DRUMSTICK "Build a static library (.a) instead of dynamic (.so)" OFF)
OPTION(ENABLE_PCH "Build using precompiled headers" OFF)

MESSAGE(STATUS "Build configuration: ${CMAKE_BUILD_TYPE}")

# LIBRARY BUILD TYPE
IF(STATIC_DRUMSTICK)
    SET(BUILD_SHARED_LIBS OFF)
    MESSAGE(STATUS "Building static libraries")
ELSE(STATIC_DRUMSTICK)
    SET(BUILD_SHARED_LIBS ON)
    MESSAGE(STATUS "Building dynamic libraries")
ENDIF(STATIC_DRUMSTICK)

# Check for Qt
SET(QT_MIN_VERSION "4.4.0")
FIND_PACKAGE(Qt4 REQUIRED)
IF(QT4_FOUND)
    MESSAGE(STATUS "Found Qt4 ${QT_EDITION} version = ${QTVERSION}")
    SET(QT_USE_QTSVG TRUE)
    INCLUDE(${QT_USE_FILE})
    LINK_DIRECTORIES(${QT_LIBRARY_DIR})
    LINK_LIBRARIES(${QT_LIBRARIES})
    ADD_DEFINITIONS(-DQT_STRICT_ITERATORS)
ELSE(QT4_FOUND)
    MESSAGE(FATAL_ERROR "Qt4 not found")
ENDIF(QT4_FOUND)

# Check for PKG-CONFIG
FIND_PACKAGE(PkgConfig REQUIRED)
IF(PKG_CONFIG_FOUND)
    MESSAGE(STATUS "Program pkg-config found (${PKG_CONFIG_EXECUTABLE})")
ELSE(PKG_CONFIG_FOUND)
    MESSAGE(FATAL_ERROR "Program pkg-config not found")
ENDIF(PKG_CONFIG_FOUND)

# Initialize some variables
SET(HAVE_ALSA FALSE)

# Check for ALSA 1.0.0
PKG_CHECK_MODULES(ALSA alsa>=1.0.0)
IF(ALSA_FOUND)
    SET(HAVE_ALSA TRUE)
    ADD_DEFINITIONS(-DHAVE_ALSA)
    SET(ALSA_LIBS ${ALSA_LIBRARIES})
    LIST(APPEND ALSA_LIB_DIR ${ALSA_LIBRARY_DIRS} ${ALSA_LIBDIR})
    LIST(APPEND ALSA_INC_DIR ${ALSA_INCLUDE_DIRS} ${ALSA_INCLUDEDIR})
ELSE(ALSA_FOUND)
    MESSAGE(STATUS "Warning: ALSA library not found.")
ENDIF(ALSA_FOUND)

LINK_DIRECTORIES(
    ${QT_LIBRARY_DIR}
    ${ALSA_LIB_DIR}
    ${DRUMSTICK_BINARY_DIR}
)

INCLUDE_DIRECTORIES( 
    ${QT_INCLUDES} 
    ${ALSA_INC_DIR}
    ${DRUMSTICK_SOURCE_DIR}/library/include
)

# Check GCC for PCH support
SET(USE_PCH FALSE)
IF(ENABLE_PCH)
    FIND_PACKAGE(PCHSupport)
    IF(PCHSupport_FOUND)
        SET(USE_PCH TRUE)
        ADD_DEFINITIONS(-DUSE_PCH)
        MESSAGE(STATUS "Enabling precompiled headers for GCC ${gcc_compiler_version}") 
    ENDIF(PCHSupport_FOUND)
ENDIF(ENABLE_PCH)

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

FIND_PACKAGE(Doxygen)
IF(DOXYGEN_FOUND)
    CONFIGURE_FILE(
        "${CMAKE_SOURCE_DIR}/Doxyfile.in"
        "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
        IMMEDIATE @ONLY)
    ADD_CUSTOM_TARGET ( doxygen
        ${DOXYGEN} Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
ENDIF(DOXYGEN_FOUND)

ADD_SUBDIRECTORY(library)
ADD_SUBDIRECTORY(tests)
ADD_SUBDIRECTORY(icons)

CONFIGURE_FILE(drumstick-alsa.pc.in drumstick-alsa.pc IMMEDIATE @ONLY)
CONFIGURE_FILE(drumstick-file.pc.in drumstick-file.pc IMMEDIATE @ONLY)
CONFIGURE_FILE(drumstick.spec.in drumstick.spec IMMEDIATE @ONLY)

# uninstall custom target
CONFIGURE_FILE(
    "${CMAKE_SOURCE_DIR}/cmake_admin/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET ( uninstall
    "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

# tarball target
ADD_CUSTOM_TARGET ( tarball
    COMMAND mkdir -p drumstick-${VERSION}
    COMMAND cp -r cmake_admin drumstick-${VERSION}
    COMMAND cp -r library drumstick-${VERSION}
    COMMAND cp -r tests drumstick-${VERSION}
    COMMAND cp -r doc drumstick-${VERSION}
    COMMAND cp -r icons drumstick-${VERSION}
    COMMAND cp CMakeLists.txt AUTHORS COPYING ChangeLog INSTALL NEWS README TODO drumstick*.in drumstick.pro drumstick.xml configure* Doxyfile.in link-drumstick chkcoverage drumstick-${VERSION}
    COMMAND tar -cj --exclude CVS --exclude .cvsignore -f drumstick-${VERSION}.tar.bz2 drumstick-${VERSION}
    COMMAND tar -cz --exclude CVS --exclude .cvsignore -f drumstick-${VERSION}.tar.gz drumstick-${VERSION}
    COMMAND rm -rf drumstick-${VERSION}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

INSTALL( FILES drumstick.xml DESTINATION share/mime/packages ) 

INSTALL( FILES 
         ${CMAKE_CURRENT_BINARY_DIR}/drumstick-file.pc
         ${CMAKE_CURRENT_BINARY_DIR}/drumstick-alsa.pc
         DESTINATION ${LIB}/pkgconfig )
