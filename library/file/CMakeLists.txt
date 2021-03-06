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
#if (APPLE)
#    set(CMAKE_MACOSX_RPATH 1)
#    set(MACOSX_FRAMEWORK_IDENTIFIER “net.sourceforge”)
#    set(MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})
#    set(MACOSX_FRAMEWORK_BUNDLE_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})
#endif()

if (QT_VERSION VERSION_GREATER_EQUAL 6.0)
    find_package(Qt6 COMPONENTS Core5Compat REQUIRED)
endif()

set(drumstick-file_QTOBJ_SRCS
    ../include/drumstick/qsmf.h
    ../include/drumstick/qwrk.h
)

set(drumstick-file_HEADERS
    ../include/drumstick/macros.h
    ../include/drumstick/qsmf.h
    ../include/drumstick/qwrk.h
)

if(APPLE)
    set_source_files_properties(${drumstick-file_HEADERS}
        PROPERTIES MACOSX_PACKAGE_LOCATION Headers/drumstick
)
endif()

set(drumstick-file_SRCS
    qsmf.cpp
    qwrk.cpp
)

if (QT_VERSION VERSION_LESS 5.15.0)
    qt5_wrap_cpp(drumstick-file_MOC_SRCS ${drumstick-file_QTOBJ_SRCS})
else()
    qt_wrap_cpp(drumstick-file_MOC_SRCS ${drumstick-file_QTOBJ_SRCS})
endif()

add_library(drumstick-file
    ${drumstick-file_MOC_SRCS}
    ${drumstick-file_SRCS} 
    ${drumstick-file_HEADERS} 
)

add_library(Drumstick::File ALIAS drumstick-file)

target_include_directories(drumstick-file PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/library/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(drumstick-file PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

if (QT_VERSION VERSION_GREATER_EQUAL 6.0)
    target_link_libraries(drumstick-file PRIVATE
        Qt6::Core5Compat
    )
endif()


if(STATIC_DRUMSTICK)
    set_target_properties(drumstick-file PROPERTIES
        STATIC_LIB "libdrumstick-file")
else()
    set_target_properties(drumstick-file PROPERTIES
        FRAMEWORK TRUE
        VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        FRAMEWORK_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
        MACOSX_FRAMEWORK_IDENTIFIER "net.sourceforge.drumstick-file"
        MACOSX_RPATH TRUE
        EXPORT_NAME File
    )
endif()

install(TARGETS drumstick-file
    EXPORT drumstick-file-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES ${drumstick-file_HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/drumstick)

install(EXPORT drumstick-file-targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/drumstick
    NAMESPACE Drumstick::
)

export(EXPORT drumstick-file-targets
    NAMESPACE Drumstick::
    FILE ${CMAKE_BINARY_DIR}/drumstick-file-targets.cmake
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(${CMAKE_BINARY_DIR}/drumstick-file-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_file( drumstick-file-config.cmake
  ${CMAKE_BINARY_DIR}/drumstick-file-config.cmake
  @ONLY
)

install(FILES
    ${CMAKE_BINARY_DIR}/drumstick-file-config-version.cmake
    ${CMAKE_BINARY_DIR}/drumstick-file-config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/drumstick
)
