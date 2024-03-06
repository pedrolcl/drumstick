#[===========================================================================[
MIDI C++ Library
Copyright (C) 2005-2024 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>

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

set ( SOURCES rttest.cpp )

add_executable ( rtTest ${SOURCES} )

target_include_directories (rtTest PUBLIC
    ${CMAKE_SOURCE_DIR}/library/include )

target_link_libraries (rtTest PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    Drumstick::RT )


if ((EXISTS "/dev/snd/") AND (EXISTS "/dev/snd/seq"))

    add_test (rtTest ${PROJECT_BINARY_DIR}/bin/rtTest)

endif()

if(STATIC_DRUMSTICK)
    if (FALSE)
        target_compile_definitions(rtTest PUBLIC DUMMY_BACKEND)
        target_link_libraries(rtTest PRIVATE
            drumstick-rt-dummy-in
            drumstick-rt-dummy-out)
    endif()

    if(ALSA_FOUND)
        target_compile_definitions(rtTest PUBLIC LINUX_BACKEND)
        target_link_libraries(rtTest PRIVATE
            drumstick-rt-alsa-in
            drumstick-rt-alsa-out
            drumstick-rt-eassynth)
    endif()

    if(UNIX AND NOT APPLE)
        target_compile_definitions(rtTest PUBLIC OSS_BACKEND)
        target_link_libraries(rtTest PRIVATE
            drumstick-rt-oss-in
            drumstick-rt-oss-out)
    endif()


    if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        target_compile_definitions(rtTest PUBLIC MAC_BACKEND)
        target_link_libraries(rtTest PRIVATE
            drumstick-rt-mac-in
            drumstick-rt-mac-out
            drumstick-rt-macsynth
            "-framework CoreMIDI -framework CoreFoundation")
    endif()

    if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        target_compile_definitions(rtTest PUBLIC WIN_BACKEND)
        target_link_libraries(rtTest PRIVATE
            drumstick-rt-win-in
            drumstick-rt-win-out
            winmm)
    endif()

    find_package(Qt${QT_VERSION_MAJOR}Network)
    if(Qt${QT_VERSION_MAJOR}Network_FOUND)
        target_compile_definitions(rtTest PUBLIC NET_BACKEND)
        target_link_libraries(rtTest PRIVATE
            drumstick-rt-net-in
            drumstick-rt-net-out)
    endif()

    if(FLUIDSYNTH_FOUND)
        target_compile_definitions(rtTest PUBLIC FLUIDSYNTH_BACKEND)
        target_link_libraries(rtTest PRIVATE drumstick-rt-fluidsynth)
    endif()
endif()
