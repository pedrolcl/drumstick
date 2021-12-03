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

set(drumstick-file_QTOBJ_SRCS
    ../include/drumstick/qsmf.h
    ../include/drumstick/qwrk.h
    ../include/drumstick/rmid.h
)

set(drumstick-file_HEADERS
    ../include/drumstick/macros.h
    ../include/drumstick/qsmf.h
    ../include/drumstick/qwrk.h
    ../include/drumstick/rmid.h
)

if(BUILD_FRAMEWORKS)
    set_source_files_properties(${drumstick-file_HEADERS}
        PROPERTIES MACOSX_PACKAGE_LOCATION Headers/drumstick
    )
endif()

set(drumstick-file_SRCS
    qsmf.cpp
    qwrk.cpp
    rmid.cpp
)

if (WIN32)
    set(TARGET_DESCRIPTION ${Drumstick_DESCRIPTION})
    set(TARGET_NAME drumstick-file)
    set(TARGET_ORIGINAL_FILENAME libdrumstick-file.dll)
    configure_file(${Drumstick_SOURCE_DIR}/versioninfo.rc.in versioninfo.rc @ONLY)
    list(APPEND drumstick-file_SRCS versioninfo.rc)
endif()

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
        VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        MACOSX_RPATH TRUE
        EXPORT_NAME File
    )
    if(BUILD_FRAMEWORKS)
        set_target_properties(drumstick-file PROPERTIES
            FRAMEWORK ${BUILD_FRAMEWORKS}
            FRAMEWORK_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
            MACOSX_FRAMEWORK_IDENTIFIER "net.sourceforge.drumstick-file"
            MACOSX_FRAMEWORK_INFO_PLIST "${CMAKE_SOURCE_DIR}/cmake_admin/CustomFrameworkInfo.plist.in"
        )
    endif()
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
