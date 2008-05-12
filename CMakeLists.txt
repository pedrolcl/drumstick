# MIDI Sequencer C++ Library
# Copyright (C) 2005-2008 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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

# The name of our project is "ASEQMM". CMakeLists files in this project can
# refer to the root source directory of the project as ${ASEQMM_SOURCE_DIR} 
# and to the root binary directory of the project as ${ASEQMM_BINARY_DIR}
PROJECT(ASEQMM)

# CMake minimum version
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.4)

# Show full commands and messages. 
# Necessary for Eclipse warning/error parser
IF(NOT CMAKE_VERBOSE_MAKEFILE)
    SET(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL
	"If this value is on all commands will be echoed to the console during the make."
    FORCE)
ENDIF(NOT CMAKE_VERBOSE_MAKEFILE)

SET(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH
	"Install path prefix, prepended onto install directories." FORCE)

#SET(EXECUTABLE_OUTPUT_PATH "bin" CACHE PATH
#    "Single output directory for building all executables." FORCE)

#SET(LIBRARY_OUTPUT_PATH "lib" CACHE PATH
#	"Single output directory for building all libraries." FORCE)
 
# Seems that relative paths break the link scripts. Don't set it ON
SET(CMAKE_USE_RELATIVE_PATHS OFF)

# Assorted settings
SET(CMAKE_COLOR_MAKEFILE ON)
SET(CMAKE_INCLUDE_CURRENT_DIR TRUE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
SET(CMAKE_SKIP_RULE_DEPENDENCY FALSE)

# Additional CMake modules for 3rd party library checks reside here
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_admin")

# ASEQMM version string
SET (VERSION_MAJOR "0")
SET (VERSION_MINOR "0")
SET (VERSION_PATCH "1")
SET (VERSION_SUFFIX "test")
SET (VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${VERSION_SUFFIX})
ADD_DEFINITIONS(-D'VERSION="${VERSION}"')

MESSAGE(STATUS "aseqmm ${VERSION} prefix: ${CMAKE_INSTALL_PREFIX}")

# User options
OPTION(WANT_DEBUG "Include debug support (source code line numbers and function names)" ON)
OPTION(WANT_STATIC "Build a static library (.a) instead of dynamic (.so)" ON)

# BUILD SETUP: None, Debug, Release, .. or custom ones
IF(WANT_DEBUG)
    SET(CMAKE_BUILD_TYPE Debug CACHE STRING 
		"Choose the type of build, options are: Debug Release" FORCE)
    SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -Wall -fexceptions")
    SET(CMAKE_C_FLAGS_DEBUG "-O0 -g3 -Wall")
ELSE(WANT_DEBUG)
    SET(CMAKE_BUILD_TYPE Release CACHE STRING
		"Choose the type of build, options are: Debug Release" FORCE)
    SET(CMAKE_CXX_FLAGS_RELEASE "-O2 -w -fexceptions")
    SET(CMAKE_C_FLAGS_RELEASE "-O2 -w")
ENDIF(WANT_DEBUG)
MESSAGE(STATUS "Build configuration: ${CMAKE_BUILD_TYPE}")

# LIBRARY BUILD TYPE
IF(WANT_STATIC)
    SET(BUILD_SHARED_LIBS OFF)
    MESSAGE(STATUS "Building static libraries")
ELSE(WANT_STATIC)
    SET(BUILD_SHARED_LIBS ON)
    MESSAGE(STATUS "Building dynamic libraries")
ENDIF(WANT_STATIC)

# Check for Qt3
FIND_PACKAGE(Qt3 REQUIRED QUIET)
IF(QT_FOUND)
    ADD_DEFINITIONS(${QT_DEFINITIONS})
    MESSAGE(STATUS "Found Qt3 version ${qt_version_str}")
    #MESSAGE("QT_DEFINITIONS: ${QT_DEFINITIONS}")
    #MESSAGE("QT_LIBRARIES: ${QT_LIBRARIES}")
    #MESSAGE("QT_WRAP_CPP: ${QT_WRAP_CPP}")
    #MESSAGE("QTDIR: $ENV{QTDIR}")
    #MESSAGE("QT_MOC_EXECUTABLE: ${QT_MOC_EXECUTABLE}")
    #MESSAGE("QT_UIC_EXECUTABLE: ${QT_UIC_EXECUTABLE}")
    #MESSAGE("QT_QT_LIBRARY: ${QT_QT_LIBRARY}")
    #MESSAGE("QT_INCLUDE_DIR: ${QT_INCLUDE_DIR}")
    #MESSAGE("qt_version_str: ${qt_version_str}")
    #MESSAGE("qt_version_str_lib: ${qt_version_str_lib}")
ELSE(QT_FOUND)
    MESSAGE(FATAL_ERROR "Qt3 not found")
ENDIF(QT_FOUND)

# Check for PKG-CONFIG
FIND_PACKAGE(PkgConfig REQUIRED)
IF(PKG_CONFIG_FOUND)
    MESSAGE(STATUS "Program pkg-config found (${PKG_CONFIG_EXECUTABLE})")
ELSE(PKG_CONFIG_FOUND)
    MESSAGE(FATAL_ERROR "Program pkg-config not found")
ENDIF(PKG_CONFIG_FOUND)

# Initialize some variables
SET(HAVE_ALSA FALSE)

# Check for ALSA 1.0
PKG_CHECK_MODULES(ALSA REQUIRED alsa>=1.0)
IF(ALSA_FOUND)
    SET(HAVE_ALSA TRUE)
    ADD_DEFINITIONS(-DHAVE_ALSA)
    SET(ALSA_LIBS ${ALSA_LIBRARIES})
    #MESSAGE("ALSA_LIBS: ${ALSA_LIBS}")
    LIST(APPEND ALSA_LIB_DIR ${ALSA_LIBRARY_DIRS} ${ALSA_LIBDIR})
    #MESSAGE("ALSA_LIB_DIR: ${ALSA_LIB_DIR}")
    LIST(APPEND ALSA_INC_DIR ${ALSA_INCLUDE_DIRS} ${ALSA_INCLUDEDIR})
    #MESSAGE("ALSA_INC_DIR: ${ALSA_INC_DIR}")
ELSE(ALSA_FOUND)
    MESSAGE(FATAL_ERROR "Please install the required package. Aborting")
ENDIF(ALSA_FOUND)

LINK_DIRECTORIES(
    ${ALSA_LIB_DIR}
    ${ASEQMM_BINARY_DIR}
)

INCLUDE_DIRECTORIES( 
    ${QT_INCLUDE_DIR} 
    ${ALSA_INC_DIR}
    ${ASEQMM_SOURCE_DIR}/library/include
)

ADD_SUBDIRECTORY(library build)
ADD_SUBDIRECTORY(tests build)
#ADD_SUBDIRECTORY(tests build EXCLUDE_FROM_ALL)

# uninstall custom target
CONFIGURE_FILE(
    "${CMAKE_SOURCE_DIR}/cmake_admin/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET ( uninstall
    "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

# Custom target trying to clean the CMake generated files
ADD_CUSTOM_TARGET ( distclean
    COMMAND rm -f  {,library/,tests/}CMakeCache.txt
    COMMAND rm -f  {,library/,tests/}cmake_install.cmake
    COMMAND rm -f  {,library/,tests/}cmake_uninstall.cmake
    COMMAND rm -f  {,library/,tests/}CPackConfig.cmake
    COMMAND rm -f  {,library/,tests/}CPackSourceConfig.cmake
    COMMAND rm -f  {,library/,tests/}install_manifest.txt
    COMMAND rm -f  {,library/,tests/}progress.make
    COMMAND rm -f  {,library/,tests/}Makefile
    COMMAND rm -rf {,library/,tests/}CMakeFiles
    COMMAND rm -rf {,library/,tests/}_CPack_Packages
    COMMAND rm -rf build/*
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
ADD_DEPENDENCIES(distclean clean)

# tarball target
ADD_CUSTOM_TARGET ( tarball
    COMMAND mkdir -p aseqmm-${VERSION}
    COMMAND cp -r cmake_admin aseqmm-${VERSION}
    COMMAND cp -r doc aseqmm-${VERSION}
    COMMAND cp -r library aseqmm-${VERSION}
    COMMAND cp -r tests aseqmm-${VERSION}
    COMMAND cp CMakeLists.txt AUTHORS COPYING ChangeLog INSTALL NEWS README TODO *.spec* aseqmm-${VERSION}
    COMMAND tar -cj --exclude CVS --exclude .cvsignore -f aseqmm-${VERSION}.tar.bz2 aseqmm-${VERSION}
    COMMAND tar -cz --exclude CVS --exclude .cvsignore -f aseqmm-${VERSION}.tar.gz aseqmm-${VERSION}
    COMMAND rm -rf aseqmm-${VERSION}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

INSTALL(FILES 
	    AUTHORS
	    COPYING
	    ChangeLog
	    NEWS
	    README
	    TODO
	DESTINATION 
	    share/docs/aseqmm	
)
