# MIDI Sequencer C++ Library
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

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(drumstick-rt_QOBJ_SRCS
    ../include/drumstick/rtmidiinput.h
    ../include/drumstick/rtmidioutput.h
)

set(drumstick-rt_HEADERS
    ../include/drumstick/macros.h
    ../include/drumstick/rtmidiinput.h
    ../include/drumstick/rtmidioutput.h
    ../include/drumstick/backendmanager.h
)

if(APPLE)
    set_source_files_properties(${drumstick-rt_HEADERS}
        PROPERTIES MACOSX_PACKAGE_LOCATION Headers/drumstick
    )
endif()

set(drumstick-rt_SRCS
    backendmanager.cpp
)

if (QT_VERSION VERSION_LESS 5.15.0)
    qt5_wrap_cpp(drumstick-rt_MOC_SRCS ${drumstick-rt_QOBJ_SRCS})
else()
    qt_wrap_cpp(drumstick-rt_MOC_SRCS ${drumstick-rt_QOBJ_SRCS})
endif()

add_library(drumstick-rt
    ${drumstick-rt_MOC_SRCS}
    ${drumstick-rt_SRCS}
    ${drumstick-rt_HEADERS}
)

add_library(Drumstick::RT ALIAS drumstick-rt)

target_include_directories(drumstick-rt PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/library/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_definitions(drumstick-rt
    PRIVATE LIBSUFFIX=${CMAKE_INSTALL_LIBDIR}
)

target_link_libraries(drumstick-rt PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

if(STATIC_DRUMSTICK)
    set_target_properties(drumstick-rt PROPERTIES
        STATIC_LIB "libdrumstick-rt")
else() # STATIC_DRUMSTICK
    set_target_properties(drumstick-rt PROPERTIES
        VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        EXPORT_NAME RT
      # macOS:
        FRAMEWORK TRUE
        FRAMEWORK_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
        MACOSX_FRAMEWORK_IDENTIFIER "net.sourceforge.drumstick-rt"
        MACOSX_RPATH TRUE
    )
endif() # STATIC_DRUMSTICK

install(TARGETS drumstick-rt
    EXPORT drumstick-rt-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES ${drumstick-rt_HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/drumstick)

install(EXPORT drumstick-rt-targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/drumstick
    NAMESPACE Drumstick::
)

export(EXPORT drumstick-rt-targets
    NAMESPACE Drumstick::
    FILE ${CMAKE_BINARY_DIR}/drumstick-rt-targets.cmake
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(${CMAKE_BINARY_DIR}/drumstick-rt-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_file( drumstick-rt-config.cmake
  ${CMAKE_BINARY_DIR}/drumstick-rt-config.cmake
  @ONLY
)

install(FILES
    ${CMAKE_BINARY_DIR}/drumstick-rt-config-version.cmake
    ${CMAKE_BINARY_DIR}/drumstick-rt-config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/drumstick
)
