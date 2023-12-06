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

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DQT_NO_DEBUG_OUTPUT")

set( HEADERS
    synthcontroller.h
    synthrenderer.h
)

set( SOURCES
    synthcontroller.cpp
    synthrenderer.cpp
    filewrapper.cpp
)

if (QT_VERSION VERSION_LESS 5.15.0)
    qt5_wrap_cpp( MOC_SRCS ${HEADERS}
        OPTIONS -I ${Drumstick_SOURCE_DIR}/library/include)
else()
    qt_wrap_cpp( MOC_SRCS ${HEADERS}
        OPTIONS -I ${Drumstick_SOURCE_DIR}/library/include)
endif()

if(STATIC_DRUMSTICK)
    add_library( drumstick-rt-eassynth STATIC ${MOC_SRCS} ${SOURCES} ${HEADERS})
    target_compile_definitions(drumstick-rt-eassynth
        PRIVATE QT_STATICPLUGIN)
    set_target_properties(drumstick-rt-eassynth PROPERTIES
         STATIC_LIB "libdrumstick-rt-eassynth")
else()
    add_library( drumstick-rt-eassynth MODULE ${MOC_SRCS} ${SOURCES} ${HEADERS})
    target_compile_definitions(drumstick-rt-eassynth
        PRIVATE QT_PLUGIN)
endif()

target_link_libraries( drumstick-rt-eassynth PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    PkgConfig::PULSE
    sonivox::sonivox
    Drumstick::RT
)

target_include_directories(drumstick-rt-eassynth PRIVATE
    ${Drumstick_SOURCE_DIR}/library/include
)

set_target_properties(drumstick-rt-eassynth PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/${DRUMSTICK_PLUGINS_DIR})

install( TARGETS drumstick-rt-eassynth
    EXPORT drumstick-rt-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/${DRUMSTICK_PLUGINS_DIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${DRUMSTICK_PLUGINS_DIR} )
