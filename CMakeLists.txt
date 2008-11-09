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
IF(COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

# Show full commands and messages. 
# SET(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL

#SET(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH
#	"Install path prefix, prepended onto install directories." FORCE)

#SET(EXECUTABLE_OUTPUT_PATH "bin" CACHE PATH
#    "Single output directory for building all executables." FORCE)

#SET(LIBRARY_OUTPUT_PATH "lib" CACHE PATH
#	"Single output directory for building all libraries." FORCE)

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

# ASEQMM version string
SET (VERSION_MAJOR "0")
SET (VERSION_MINOR "0")
SET (VERSION_PATCH "2")
SET (VERSION_SUFFIX "pre2")
SET (VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_SUFFIX})
ADD_DEFINITIONS(-D'VERSION="${VERSION}"')

MESSAGE(STATUS "aseqmm ${VERSION} prefix: ${CMAKE_INSTALL_PREFIX}")

# User options
OPTION(WANT_STATIC "Build a static library (.a) instead of dynamic (.so)" OFF)

MESSAGE(STATUS "Build configuration: ${CMAKE_BUILD_TYPE}")

# LIBRARY BUILD TYPE
IF(WANT_STATIC)
    SET(BUILD_SHARED_LIBS OFF)
    MESSAGE(STATUS "Building static libraries")
ELSE(WANT_STATIC)
    SET(BUILD_SHARED_LIBS ON)
    MESSAGE(STATUS "Building dynamic libraries")
ENDIF(WANT_STATIC)

# Check for Qt
FIND_PACKAGE(Qt4 REQUIRED QUIET)
IF(QT4_FOUND)
    ADD_DEFINITIONS(${QT_DEFINITIONS})
    ADD_DEFINITIONS("-DQT_GUI_LIB -DQT_CORE_LIB")
    IF (NOT QT_LIBRARIES)
	SET(QT_LIBRARIES ${QT_QTCORE_LIBRARY}
			 ${QT_QTGUI_LIBRARY} 
			 ${QT_CORE_LIB_DEPENDENCIES} 
			 ${QT_GUI_LIB_DEPENDENCIES})
    ENDIF (NOT QT_LIBRARIES)    
    MESSAGE(STATUS "Found Qt4 ${QT_EDITION} version = ${QTVERSION}")
    #MESSAGE("QT_DEFINITIONS: ${QT_DEFINITIONS}")
    #MESSAGE("QT_LIBRARIES: ${QT_LIBRARIES}")
    #MESSAGE("QT_QTCORE_LIBRARY: ${QT_QTCORE_LIBRARY}")
    #MESSAGE("QT_QTGUI_LIBRARY: ${QT_QTGUI_LIBRARY}")
    #MESSAGE("QT_INCLUDES: ${QT_INCLUDES}")
    #MESSAGE("QT_WRAP_CPP: ${QT_WRAP_CPP}")
    #MESSAGE("QT_MOC_EXECUTABLE: ${QT_MOC_EXECUTABLE}")
    #MESSAGE("QT_UIC_EXECUTABLE: ${QT_UIC_EXECUTABLE}")
    #MESSAGE("QT_LIBRARY_DIR: ${QT_LIBRARY_DIR}")
    #MESSAGE("QT_INCLUDE_DIR: ${QT_INCLUDE_DIR}")
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
PKG_CHECK_MODULES(ALSA REQUIRED alsa>=1.0.0)
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
    ${QT_LIBRARY_DIR}
    ${ALSA_LIB_DIR}
    ${ASEQMM_BINARY_DIR}
)

INCLUDE_DIRECTORIES( 
    ${QT_INCLUDES} 
    ${ALSA_INC_DIR}
    ${ASEQMM_SOURCE_DIR}/library/include
)

ADD_SUBDIRECTORY(library build/library)
ADD_SUBDIRECTORY(tests build/tests)

CONFIGURE_FILE(aseqmm.pc.in aseqmm.pc IMMEDIATE @ONLY)
CONFIGURE_FILE(aseqmm.spec.in aseqmm.spec IMMEDIATE @ONLY)

# uninstall custom target
CONFIGURE_FILE(
    "${CMAKE_SOURCE_DIR}/cmake_admin/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET ( uninstall
    "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

# Custom target trying to clean the CMake generated files
ADD_CUSTOM_TARGET ( distclean
    COMMAND rm -f  aseqmm.pc aseqmm.spec
    COMMAND rm -f  {,library/,tests/}CMakeCache.txt
    COMMAND rm -f  {,library/,tests/}cmake_install.cmake
    COMMAND rm -f  {,library/,tests/}cmake_uninstall.cmake
    COMMAND rm -f  {,library/,tests/}CPackConfig.cmake
    COMMAND rm -f  {,library/,tests/}CPackSourceConfig.cmake
    COMMAND rm -f  {,library/,tests/}install_manifest.txt
    COMMAND rm -f  {,library/,tests/}progress.make
    COMMAND rm -f  {,library/,tests/}Makefile
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
    COMMAND cp CMakeLists.txt AUTHORS COPYING ChangeLog INSTALL NEWS README TODO aseqmm.spec.in aseqmm.pc.in aseqmm-${VERSION}
    COMMAND tar -cj --exclude CVS --exclude .cvsignore -f aseqmm-${VERSION}.tar.bz2 aseqmm-${VERSION}
    COMMAND tar -cz --exclude CVS --exclude .cvsignore -f aseqmm-${VERSION}.tar.gz aseqmm-${VERSION}
    COMMAND rm -rf aseqmm-${VERSION}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

#INSTALL(FILES 
#	    AUTHORS
#	    COPYING
#	    ChangeLog
#	    NEWS
#	    README
#	    TODO
#	DESTINATION 
#	    share/doc/packages/aseqmm	
#)

INSTALL(FILES aseqmm.pc
        DESTINATION ${LIB}/pkgconfig)
