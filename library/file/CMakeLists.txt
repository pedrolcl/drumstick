# MIDI Sequencer C++ Library
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

set(CMAKE_INCLUDE_CURRENT_DIR ON)
if (APPLE)
    set(CMAKE_MACOSX_RPATH 1)
    set(MACOSX_FRAMEWORK_IDENTIFIER “net.sourceforge”)
    set(MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
    set(MACOSX_FRAMEWORK_BUNDLE_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
endif()

set(drumstick-file_QTOBJ_SRCS
    ../include/drumstick/qsmf.h
    ../include/drumstick/qwrk.h
    ../include/drumstick/qove.h
)

set(drumstick-file_HEADERS
    ../include/drumstick/macros.h
    ../include/drumstick/qsmf.h
    ../include/drumstick/qwrk.h
    ../include/drumstick/qove.h
)

set(drumstick-file_SRCS
    qsmf.cpp
    qwrk.cpp
    qove.cpp
)

qt5_wrap_cpp(drumstick-file_MOC_SRCS ${drumstick-file_QTOBJ_SRCS})

add_library(drumstick-file
    ${drumstick-file_MOC_SRCS}
    ${drumstick-file_SRCS} 
    ${drumstick-file_HEADERS} 
)

add_library(Drumstick::File ALIAS drumstick-file)

target_include_directories(drumstick-file PUBLIC
    $<BUILD_INTERFACE:${Drumstick_SOURCE_DIR}/library/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDE}>
)

target_link_libraries(drumstick-file
    PUBLIC Qt5::Core
)

if(STATIC_DRUMSTICK)
    set_target_properties(drumstick-file PROPERTIES
        STATIC_LIB "libdrumstick-file")
else()
    set_target_properties(drumstick-file PROPERTIES
        FRAMEWORK TRUE
        VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
        SOVERSION ${PROJECT_VERSION_MAJOR}
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
    COMPONENT File
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(${CMAKE_BINARY_DIR}/drumstick-file-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    ${CMAKE_BINARY_DIR}/drumstick-file-config-version.cmake
    drumstick-file-config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/drumstick
    COMPONENT File
)
