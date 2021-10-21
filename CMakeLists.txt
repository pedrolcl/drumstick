# MIDI C++ Library
# Copyright (C) 2005-2021 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.14)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum OS X deployment version")

project( Drumstick
    VERSION 2.4.1
    LANGUAGES C CXX
    DESCRIPTION "MIDI C++ Libraries for Qt"
    HOMEPAGE_URL "https://sourceforge.net/p/drumstick/"
)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_admin)
include(SCMRevision)
include(CTest)
include(GNUInstallDirs)

#set(RELEASE_DATE "September 19, 2021")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# cmake bug https://gitlab.kitware.com/cmake/cmake/issues/18396 closed in cmake 3.14
if(APPLE AND ${CMAKE_VERSION} VERSION_LESS 3.14)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

if (WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8") #/source-charset:utf-8 /execution-charset:utf-8
endif()

if(UNIX AND NOT APPLE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
        #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic")
        #set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--as-needed")
        #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed")
        #set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--as-needed -Wl,--no-undefined")
    endif()
endif()

add_definitions(-DVERSION=${PROJECT_VERSION})
if (DEFINED PROJECT_WC_REVISION)
    add_definitions(-DREVISION=${PROJECT_WC_REVISION})
endif()

if (UNIX)
    set(_DOCS_INIT ON)
else()
    set(_DOCS_INIT OFF)
endif()

set(_DBUS_INIT OFF)
if (UNIX AND NOT APPLE)
    set(_PULSE_INIT ON)
else ()
    set(_PULSE_INIT OFF)
endif ()

if (APPLE)
    set(_FRAMEWORKS_INIT ON)
else()
    set(_FRAMEWORKS_INIT OFF)
endif()

option(BUILD_DOCS "Build Doxygen documentation and man pages" ${_DOCS_INIT})
option(BUILD_UTILS "Build Utilities and Examples" ON)
option(BUILD_FRAMEWORKS "Build macOS style frameworks" ${_FRAMEWORKS_INIT})
# set CMAKE_MACOSX_BUNDLE OFF in macOS to avoid bundles
option(STATIC_DRUMSTICK "Build static libraries instead of dynamic" OFF)
option(USE_DBUS "Include DBus support (required for RealtimeKit)" ${_DBUS_INIT})
option(USE_PULSEAUDIO "Build Sonivox RT backend (if PulseAudio is available)" ${_PULSE_INIT})
option(USE_FLUIDSYNTH "Build FluidSynth RT backend (if available)" ON)
option(USE_NETWORK "Build Network RT backend (if QtNetwork is available)" ON)

if(STATIC_DRUMSTICK)
    set(BUILD_SHARED_LIBS OFF)
    add_definitions(-DDRUMSTICK_STATIC)
    message(STATUS "Building static libraries")
else()
    set(BUILD_SHARED_LIBS ON)
    if (BUILD_FRAMEWORKS)
        message(STATUS "Building macOS style frameworks")
    else()
        message(STATUS "Building unix dynamic libraries")
    endif()
endif()

if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.15)
    set(CMAKE_FRAMEWORK ${BUILD_FRAMEWORKS})
endif()

find_package(QT NAMES Qt5 Qt6 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)
message(STATUS "Found Qt version = ${QT_VERSION}")

if(USE_DBUS)
    find_package(Qt${QT_VERSION_MAJOR}DBus REQUIRED)
    add_definitions(-DRTKIT_SUPPORT)
    message(STATUS "Found QtDBus version = ${Qt${QT_VERSION_MAJOR}DBus_VERSION}")
endif()

if(USE_NETWORK)
    find_package(Qt${QT_VERSION_MAJOR}Network REQUIRED)
    if(Qt${QT_VERSION_MAJOR}Network_FOUND)
        message(STATUS "Found QtNetwork version = ${Qt${QT_VERSION_MAJOR}Network_VERSION}")
    else()
        message(STATUS "Warning: QtNetwork library not available. Network RT backend won't be built.")
    endif()
endif()

find_package(PkgConfig REQUIRED)
if(PKG_CONFIG_FOUND)
	message(STATUS "Program pkg-config ${PKG_CONFIG_VERSION_STRING} found (${PKG_CONFIG_EXECUTABLE})")
else()
	message(FATAL_ERROR "Program pkg-config not found")
endif()

if(${CMAKE_SYSTEM} MATCHES "Linux")
    pkg_check_modules(ALSA REQUIRED IMPORTED_TARGET alsa>=1.0.0)
    if(ALSA_FOUND)
        message(STATUS "Found ALSA version = ${ALSA_VERSION}")
    endif()
endif()

if(UNIX AND NOT APPLE)
    if (USE_PULSEAUDIO)
        pkg_check_modules(PULSE QUIET IMPORTED_TARGET libpulse-simple)
    endif()
    if(PULSE_FOUND)
        message(STATUS "Found PulseAudio version = ${PULSE_VERSION}")
    else()
        message(STATUS "Warning: PulseAudio library not available. Sonivox RT backend won't be built.")
    endif()
endif()

if (PKG_CONFIG_FOUND)
    if (USE_FLUIDSYNTH)
        pkg_check_modules(FLUIDSYNTH QUIET IMPORTED_TARGET fluidsynth>=1.1.1)
    endif()
    if(FLUIDSYNTH_FOUND)
        message(STATUS "Found FluidSynth version = ${FLUIDSYNTH_VERSION}")
    else()
        message(STATUS "Warning: FluidSynth library not available. FluidSynth RT backend won't be built.")
    endif()
endif()

if (${CMAKE_SYSTEM} MATCHES "Darwin")
    find_package(Qt${QT_VERSION_MAJOR}Concurrent REQUIRED)
    if (Qt${QT_VERSION_MAJOR}Concurrent_FOUND)
        message(STATUS "Found Qt${QT_VERSION_MAJOR}Concurrent version = ${Qt${QT_VERSION_MAJOR}Concurrent_VERSION_STRING}")
    endif()
endif()

# CMAKE_SYSTEM_PROCESSOR is broken on Windows with MSVC
# cmake bug https://gitlab.kitware.com/cmake/cmake/-/issues/15170 still open in 2021/03
if (MSVC)
    string(TOLOWER ${MSVC_CXX_ARCHITECTURE_ID} CMAKE_SYSTEM_PROCESSOR)
endif()

message (STATUS "${PROJECT_NAME} v${PROJECT_VERSION}
    Install prefix: ${CMAKE_INSTALL_PREFIX}
    Build configuration: ${CMAKE_BUILD_TYPE}
    System: ${CMAKE_SYSTEM_NAME}
    Processor: ${CMAKE_SYSTEM_PROCESSOR}
    Qt Version: ${QT_VERSION}
    D-Bus support: ${USE_DBUS}
    Network support: ${USE_NETWORK}
    PulseAudio support: ${USE_PULSEAUDIO}
    FluidSynth support: ${USE_FLUIDSYNTH}
    Building tests: ${BUILD_TESTING}
    Building docs: ${BUILD_DOCS}
    Building utils: ${BUILD_UTILS}
    Building frameworks: ${BUILD_FRAMEWORKS}"
)

add_subdirectory(library)
if (BUILD_UTILS)
    add_subdirectory(utils)
endif()
add_subdirectory(icons)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

if(UNIX AND NOT APPLE)
    if (BUILD_DOCS)
        find_package(Doxygen REQUIRED dot)
        if(DOXYGEN_FOUND)
            string(REPLACE ";" " " DOXYGEN_INCLUDE_DIRS "${Qt${QT_VERSION_MAJOR}Core_INCLUDE_DIRS}")
            configure_file("${CMAKE_SOURCE_DIR}/Doxyfile.in"
                "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile" IMMEDIATE @ONLY)
            add_custom_target(doxygen
                ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
        endif()
        add_subdirectory(doc)
    endif()
    configure_file(drumstick.spec.in drumstick.spec IMMEDIATE @ONLY)
    if(ALSA_FOUND AND PKG_CONFIG_FOUND)
        configure_file(drumstick-alsa.pc.in drumstick-alsa.pc IMMEDIATE @ONLY)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/drumstick-alsa.pc
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig )
    endif()
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
    configure_file(drumstick-widgets.pc.in drumstick-widgets.pc IMMEDIATE @ONLY)
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/drumstick-widgets.pc
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
        COMMAND cp CMakeLists.txt AUTHORS COPYING ChangeLog NEWS TODO *.md drumstick*.in drumstick.pro drumstick.xml configure* Doxyfile.in global.pri versioninfo.rc.in drumstick-${PROJECT_VERSION}
        COMMAND tar -cj --exclude=.[a-z]* -f drumstick-${PROJECT_VERSION}.tar.bz2 drumstick-${PROJECT_VERSION}
        COMMAND tar -cz --exclude=.[a-z]* -f drumstick-${PROJECT_VERSION}.tar.gz  drumstick-${PROJECT_VERSION}
        COMMAND zip -qr drumstick-${PROJECT_VERSION}.zip drumstick-${PROJECT_VERSION} -x '.[a-z]*'
        COMMAND rm -rf drumstick-${PROJECT_VERSION}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()
