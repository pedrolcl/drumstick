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

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Gui Widgets LinguistTools REQUIRED)

set(EXTRA_LIBS)
if(APPLE)
    find_library(CORELIBS CoreFoundation)
    set(EXTRA_LIBS ${CORELIBS})
endif()

set(vpiano_SRCS
    connections.cpp
    connections.h
    connections.ui
    preferences.cpp
    preferences.h
    preferences.ui
    vpiano.cpp
    vpiano.h
    vpiano.ui
    vpianoabout.cpp
    vpianoabout.h
    vpianoabout.ui
    vpianomain.cpp
    vpianosettings.cpp
    vpianosettings.h
)

if (WIN32)
    set(TARGET_DESCRIPTION "Drumstick Virtual Piano")
    set(TARGET_NAME drumstick-vpiano)
    set(TARGET_ORIGINAL_FILENAME drumstick-vpiano.exe)
    configure_file(${Drumstick_SOURCE_DIR}/versioninfo.rc.in versioninfo.rc @ONLY)
    list(APPEND vpiano_SRCS versioninfo.rc)
endif()

if (APPLE)
    set(ICON_FILE "${PROJECT_SOURCE_DIR}/icons/drumstick.icns")
    list(APPEND vpiano_SRCS ${ICON_FILE})
    set_source_files_properties (${ICON_FILE} PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )
endif()

add_executable(drumstick-vpiano
    ${vpiano_SRCS}
)

target_link_libraries(drumstick-vpiano PRIVATE
    Drumstick::RT
    Drumstick::Widgets
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    ${EXTRA_LIBS}
)

if (WIN32)
    set_target_properties( drumstick-vpiano PROPERTIES
        WIN32_EXECUTABLE TRUE )
endif()

if (APPLE)
    set_target_properties(drumstick-vpiano PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_STRING "Drumstick Virtual Piano"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_LONG_VERSION_STRING ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
        MACOSX_BUNDLE_ICON_FILE "drumstick.icns"
        MACOSX_BUNDLE_GUI_IDENTIFIER "net.sourceforge.drumstick-vpiano"
        MACOSX_BUNDLE_BUNDLE_NAME "drumstick-vpiano"
        MACOSX_BUNDLE_COPYRIGHT "© 2008-2021 Pedro López-Cabanillas"
        MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/cmake_admin/CustomBundleInfo.plist.in"
    )
endif()

set(TS_FILES
    drumstick-vpiano_en.ts
    drumstick-vpiano_es.ts
)

if (QT_VERSION VERSION_LESS 5.15.0)
    qt5_add_translation(QM_FILES ${TS_FILES})
else()
    qt_add_translation(QM_FILES ${TS_FILES})
endif()

add_custom_target(drumstick-vpiano-translations ALL DEPENDS ${QM_FILES})

if (UNIX)
    install( FILES ${QM_FILES}
             DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/drumstick )
endif ()

if(STATIC_DRUMSTICK)
    if (FALSE)
        target_compile_definitions(drumstick-vpiano PUBLIC DUMMY_BACKEND)
        target_link_libraries(drumstick-vpiano PRIVATE
            drumstick-rt-dummy-in
            drumstick-rt-dummy-out)
    endif()

    if(ALSA_FOUND)
        target_compile_definitions(drumstick-vpiano PUBLIC LINUX_BACKEND)
        target_link_libraries(drumstick-vpiano PRIVATE
            drumstick-rt-alsa-in
            drumstick-rt-alsa-out
            drumstick-rt-eassynth)
    endif()

    if(UNIX AND NOT APPLE)
        target_compile_definitions(drumstick-vpiano PUBLIC OSS_BACKEND)
        target_link_libraries(drumstick-vpiano PRIVATE
            drumstick-rt-oss-in
            drumstick-rt-oss-out)
    endif()


    if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        target_compile_definitions(drumstick-vpiano PUBLIC MAC_BACKEND)
        target_link_libraries(drumstick-vpiano PRIVATE
            drumstick-rt-mac-in
            drumstick-rt-mac-out
            drumstick-rt-macsynth
            "-framework CoreMIDI -framework CoreFoundation")
    endif()

    if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        target_compile_definitions(drumstick-vpiano PUBLIC WIN_BACKEND)
        target_link_libraries(drumstick-vpiano PRIVATE
            drumstick-rt-win-in
            drumstick-rt-win-out
            winmm)
    endif()

    find_package(Qt${QT_VERSION_MAJOR}Network)
    if(Qt${QT_VERSION_MAJOR}Network_FOUND)
        target_compile_definitions(drumstick-vpiano PUBLIC NET_BACKEND)
        target_link_libraries(drumstick-vpiano PRIVATE
            drumstick-rt-net-in
            drumstick-rt-net-out)
    endif()

    if(FLUIDSYNTH_FOUND)
        target_compile_definitions(drumstick-vpiano PUBLIC FLUIDSYNTH_BACKEND)
        target_link_libraries(drumstick-vpiano PRIVATE drumstick-rt-fluidsynth)
    endif()
endif()

install(TARGETS drumstick-vpiano
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        BUNDLE DESTINATION ~/Applications )

install(FILES drumstick-vpiano.desktop
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
