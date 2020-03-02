# Drumstick Widgets C++ Library
# Copyright (C) 2005-2020 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)

set(drumstick-widgets_HEADERS
    ../include/drumstick/pianokeybd.h
    ../include/drumstick/configurationdialogs.h
)

set(drumstick-widgets_OBJ_SRCS
    ../include/drumstick/pianokeybd.h
    pianoscene.h
    fluidsettingsdialog.h
    networksettingsdialog.h
    macsynthsettingsdialog.h
    sonivoxsettingsdialog.h
)

if(APPLE)
    set_source_files_properties(${drumstick-widgets_HEADERS}
        PROPERTIES MACOSX_PACKAGE_LOCATION Headers/drumstick
    )
endif()

set(drumstick-widgets_SRCS
    configurationdialogs.cpp
    fluidsettingsdialog.cpp
    fluidsettingsdialog.h
    keyboardmap.h
    keylabel.cpp
    keylabel.h
    macsynthsettingsdialog.cpp
    macsynthsettingsdialog.h
    networksettingsdialog.cpp
    networksettingsdialog.h
    pianodefs.h
    pianokey.cpp
    pianokey.h
    pianokeybd.cpp
    pianopalette.cpp
    pianopalette.h
    pianoscene.cpp
    pianoscene.h
    sonivoxsettingsdialog.cpp
    sonivoxsettingsdialog.h
)

set(drumstick-widgets_FORMS
    fluidsettingsdialog.ui
    macsynthsettingsdialog.ui
    networksettingsdialog.ui
    sonivoxsettingsdialog.ui
)

qt5_wrap_ui(drumstick-widgets_UI_SRCS ${drumstick-widgets_FORMS})
qt5_wrap_cpp(drumstick-widgets_MOC_SRCS ${drumstick-widgets_OBJ_SRCS})
qt5_add_resources(drumstick-widgets_RESOURCES pianokeybd.qrc)

add_library(drumstick-widgets
    ${drumstick-widgets_UI_SRCS}
    ${drumstick-widgets_MOC_SRCS}
    ${drumstick-widgets_SRCS}
    ${drumstick-widgets_RESOURCES}
    ${drumstick-widgets_HEADERS}
)

add_library(Drumstick::Widgets ALIAS drumstick-widgets)

target_include_directories(drumstick-widgets PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/library/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_definitions(drumstick-widgets
    PRIVATE LIBSUFFIX=${CMAKE_INSTALL_LIBDIR}
)

target_link_libraries(drumstick-widgets
    PUBLIC Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Network
)

if(STATIC_DRUMSTICK)
    set_target_properties(drumstick-widgets PROPERTIES
        STATIC_LIB "libdrumstick-widgets")
else() # STATIC_DRUMSTICK
    set_target_properties(drumstick-widgets PROPERTIES
        VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        EXPORT_NAME Widgets
      # macOS:
        FRAMEWORK TRUE
        FRAMEWORK_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
        MACOSX_FRAMEWORK_IDENTIFIER "net.sourceforge.drumstick-widgets"
        MACOSX_RPATH TRUE
    )
endif() # STATIC_DRUMSTICK

install(TARGETS drumstick-widgets
    EXPORT drumstick-widgets-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES ${drumstick-widgets_HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/drumstick)

install(EXPORT drumstick-widgets-targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/drumstick
    NAMESPACE Drumstick::
)

export(EXPORT drumstick-widgets-targets
    NAMESPACE Drumstick::
    FILE ${CMAKE_BINARY_DIR}/drumstick-widgets-targets.cmake
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(${CMAKE_BINARY_DIR}/drumstick-widgets-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_file( drumstick-widgets-config.cmake
  ${CMAKE_BINARY_DIR}/drumstick-widgets-config.cmake
  @ONLY
)

install(FILES
    ${CMAKE_BINARY_DIR}/drumstick-widgets-config-version.cmake
    ${CMAKE_BINARY_DIR}/drumstick-widgets-config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/drumstick
)
