#[===========================================================================[
MIDI C++ Library
Copyright (C) 2005-2023 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
#]===========================================================================]

if(BUILD_ALSA AND ALSA_FOUND)
    add_subdirectory(alsa)
endif()
if (BUILD_FILE)
    add_subdirectory(file)
endif()
if (BUILD_RT)
    add_subdirectory(rt)
    add_subdirectory(rt-backends)
    if (BUILD_WIDGETS)
        add_subdirectory(widgets)
        if (NOT STATIC_DRUMSTICK)
            add_subdirectory(vpiano-plugin)
        endif()
    endif()
endif()

configure_file( drumstick-config.cmake.in
  ${CMAKE_BINARY_DIR}/drumstick-config.cmake
  @ONLY
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file( ${CMAKE_BINARY_DIR}/drumstick-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES include/drumstick.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install( FILES
    ${CMAKE_BINARY_DIR}/drumstick-config.cmake
    ${CMAKE_BINARY_DIR}/drumstick-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/drumstick
)
