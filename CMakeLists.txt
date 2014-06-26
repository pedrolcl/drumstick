# MIDI C++ Library
# Copyright (C) 2005-2014 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)

IF(CMAKE_SIZEOF_VOID_P MATCHES "8")
    SET(_INIT_LIB_SUFFIX "64")
ELSE(CMAKE_SIZEOF_VOID_P MATCHES "8")
    SET(_INIT_LIB_SUFFIX "")
ENDIF(CMAKE_SIZEOF_VOID_P MATCHES "8")
SET(LIB_SUFFIX ${_INIT_LIB_SUFFIX} CACHE STRING "optional library install directory suffix: 32, 64, none")

#Comment out if relative paths break the link scripts
#SET(CMAKE_USE_RELATIVE_PATHS OFF)

# Assorted settings
set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
#set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_SKIP_RULE_DEPENDENCY FALSE)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# Additional CMake modules for 3rd party library checks reside here
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_admin")

# DRUMSTICK version string
SET (VERSION_MAJOR "0")
SET (VERSION_MINOR "5")
SET (VERSION_PATCH "99")
SET (VERSION_SUFFIX "svn")
SET (VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_SUFFIX})
add_definitions(-DVERSION=${VERSION})

MESSAGE(STATUS "drumstick ${VERSION} prefix: ${CMAKE_INSTALL_PREFIX}")

# User options
OPTION(STATIC_DRUMSTICK "Build static libraries instead of dynamic" OFF)
OPTION(USE_DBUS "Include DBus support (required for RealtimeKit)" ON)

MESSAGE(STATUS "Build configuration: ${CMAKE_BUILD_TYPE}")

# LIBRARY BUILD TYPE
IF(STATIC_DRUMSTICK)
    SET(BUILD_SHARED_LIBS OFF)
    add_definitions(-DDRUMSTICK_STATIC)
    MESSAGE(STATUS "Building static libraries")
ELSE(STATIC_DRUMSTICK)
    SET(BUILD_SHARED_LIBS ON)
    MESSAGE(STATUS "Building dynamic libraries")
ENDIF(STATIC_DRUMSTICK)

# Check for Qt
find_package(Qt5Core REQUIRED)
message(STATUS "Found Qt5 version = ${Qt5Core_VERSION_STRING}")
include_directories(${Qt5Core_INCLUDE_DIRS})
link_libraries(${Qt5Core_LIBRARIES})
add_definitions(${Qt5Core_DEFINITIONS})

if(USE_DBUS)
    find_package(Qt5DBus REQUIRED)
    ADD_DEFINITIONS(-DRTKIT_SUPPORT)
    include_directories(${Qt5DBus_INCLUDE_DIRS})
    link_libraries(${Qt5DBus_LIBRARIES})
    add_definitions(${Qt5DBus_DEFINITIONS})
endif()


set(CMAKE_POSITION_INDEPENDENT_CODE ON)

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
pkg_check_modules(ALSA alsa>=1.0.0)
if(ALSA_FOUND)
    SET(HAVE_ALSA TRUE)
    ADD_DEFINITIONS(-DHAVE_ALSA)
    SET(ALSA_LIBS ${ALSA_LIBRARIES})
    LIST(APPEND ALSA_LIB_DIR ${ALSA_LIBRARY_DIRS} ${ALSA_LIBDIR})
    LIST(APPEND ALSA_INC_DIR ${ALSA_INCLUDE_DIRS} ${ALSA_INCLUDEDIR})
else()
    MESSAGE(STATUS "Warning: ALSA library not found.")
endif()

link_directories(
    ${DRUMSTICK_BINARY_DIR}/lib
    ${ALSA_LIB_DIR}
)

include_directories( BEFORE
    ${DRUMSTICK_SOURCE_DIR}/library
    ${DRUMSTICK_SOURCE_DIR}/library/include
    ${ALSA_INC_DIR}
)

if(CMAKE_COMPILER_IS_GNUCXX)
# exceptions are needed by this library, please don't try to disable them!
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -UQT_NO_EXCEPTIONS")
  if(NOT WIN32)
    include (CheckCXXCompilerFlag)
    check_cxx_compiler_flag (-fvisibility=hidden _HAVE_GCC_VISIBILITY)
    if(_HAVE_GCC_VISIBILITY)
      set(CMAKE_CXX_FLAGS
           "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
    endif(_HAVE_GCC_VISIBILITY)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed")
  endif(NOT WIN32)
endif(CMAKE_COMPILER_IS_GNUCXX)

add_subdirectory(library)
add_subdirectory(utils)
add_subdirectory(icons)
if(${CMAKE_SYSTEM} MATCHES "Linux")
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
    ADD_SUBDIRECTORY(doc)
    CONFIGURE_FILE(drumstick-alsa.pc.in drumstick-alsa.pc IMMEDIATE @ONLY)
    CONFIGURE_FILE(drumstick.spec.in drumstick.spec IMMEDIATE @ONLY)
    INSTALL( FILES 
	${CMAKE_CURRENT_BINARY_DIR}/drumstick-alsa.pc
	DESTINATION lib${LIB_SUFFIX}/pkgconfig )
endif()

CONFIGURE_FILE(drumstick-file.pc.in drumstick-file.pc IMMEDIATE @ONLY)
INSTALL( FILES 
    ${CMAKE_CURRENT_BINARY_DIR}/drumstick-file.pc
    DESTINATION lib${LIB_SUFFIX}/pkgconfig )

# XML mime types
set( SHARED_MIME_INFO_MINIMUM_VERSION "0.30" )
set( XDG_MIME_INSTALL_DIR "share/mime/packages" )
find_package( SharedMimeInfo )
if( SHARED_MIME_INFO_FOUND )
    install( FILES drumstick.xml DESTINATION ${XDG_MIME_INSTALL_DIR} ) 
    update_xdg_mimetypes( ${XDG_MIME_INSTALL_DIR} )
endif( SHARED_MIME_INFO_FOUND )

# uninstall custom target
CONFIGURE_FILE(
    "${CMAKE_SOURCE_DIR}/cmake_admin/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET ( uninstall
    "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

IF(${CMAKE_SYSTEM} MATCHES "Linux")
    # tarball target
    ADD_CUSTOM_TARGET ( tarball
        COMMAND mkdir -p drumstick-${VERSION}
        COMMAND cp -r cmake_admin drumstick-${VERSION}
        COMMAND cp -r library drumstick-${VERSION}
        COMMAND cp -r utils drumstick-${VERSION}
        COMMAND cp -r doc drumstick-${VERSION}
        COMMAND cp -r icons drumstick-${VERSION}
        COMMAND cp CMakeLists.txt AUTHORS COPYING ChangeLog INSTALL NEWS README TODO drumstick*.in drumstick.pro drumstick.xml configure* Doxyfile.in link-drumstick chkcoverage drumstick-${VERSION}
        COMMAND tar -cj --exclude=.[a-z]* -f drumstick-${VERSION}.tar.bz2 drumstick-${VERSION}
        COMMAND tar -cz --exclude=.[a-z]* -f drumstick-${VERSION}.tar.gz  drumstick-${VERSION}
        COMMAND rm -rf drumstick-${VERSION}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
ENDIF(${CMAKE_SYSTEM} MATCHES "Linux")
