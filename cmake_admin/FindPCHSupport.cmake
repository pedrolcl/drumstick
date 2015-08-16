# MIDI Sequencer C++ Library 
# Copyright (C) 2008-2015 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA 02110-1301, USA

# Try to find precompiled headers support for GCC 3.4 and 4.x
# Once done this will define:
#
# Variable:
#   PCHSupport_FOUND
#
# Macro:
#   ADD_PRECOMPILED_HEADER

IF(CMAKE_COMPILER_IS_GNUCXX)
    EXEC_PROGRAM(${CMAKE_CXX_COMPILER}
                 ARGS -dumpversion
                 OUTPUT_VARIABLE gcc_compiler_version)
    IF(gcc_compiler_version MATCHES "4\\.[0-9]\\.[0-9]")
        SET(PCHSupport_FOUND TRUE)
    ELSE(gcc_compiler_version MATCHES "4\\.[0-9]\\.[0-9]")
        IF(gcc_compiler_version MATCHES "3\\.4\\.[0-9]")
            SET(PCHSupport_FOUND TRUE)
        ENDIF(gcc_compiler_version MATCHES "3\\.4\\.[0-9]")
    ENDIF(gcc_compiler_version MATCHES "4\\.[0-9]\\.[0-9]")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

MACRO(ADD_PRECOMPILED_HEADER _targetName _input)
    GET_FILENAME_COMPONENT(_name ${_input} NAME)
    SET(_source "${CMAKE_CURRENT_SOURCE_DIR}/${_input}")
    SET(_outdir "${CMAKE_CURRENT_BINARY_DIR}/${_name}.gch")
    MAKE_DIRECTORY(${_outdir})
    IF(NOT CMAKE_BUILD_TYPE)
        SET(_output "${_outdir}/Custom.c++")
    ELSE(NOT CMAKE_BUILD_TYPE)
        SET(_output "${_outdir}/${CMAKE_BUILD_TYPE}.c++")
    ENDIF(NOT CMAKE_BUILD_TYPE)
    STRING(TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" _flags_var_name)
    SET(_compiler_FLAGS ${CMAKE_CXX_FLAGS} ${${_flags_var_name}})
    SEPARATE_ARGUMENTS(_compiler_FLAGS)
    #MESSAGE("_compiler_FLAGS: ${_compiler_FLAGS}")
    GET_DIRECTORY_PROPERTY(_dir_FLAGS INCLUDE_DIRECTORIES)
    FOREACH(item ${_dir_FLAGS})
        LIST(APPEND _include_FLAGS "-I${item}")
    ENDFOREACH(item)    
    #MESSAGE("_include_FLAGS: ${_include_FLAGS}")
    GET_DIRECTORY_PROPERTY(_definitions24 DEFINITIONS)
    GET_DIRECTORY_PROPERTY(_definitions26 COMPILE_DEFINITIONS)
    SET(_definitions ${_definitions24})
    SEPARATE_ARGUMENTS(_definitions)
    FOREACH(def ${_definitions26})
        LIST(APPEND _definitions "-D${def}")
    ENDFOREACH(def)
    STRING(TOUPPER "COMPILE_DEFINITIONS_${CMAKE_BUILD_TYPE}" _defs_var_name)
    GET_DIRECTORY_PROPERTY(_definitions_buildtype ${_defs_var_name})
    FOREACH(def ${_definitions_buildtype})
        LIST(APPEND _definitions "-D${def}")
    ENDFOREACH(def)
    LIST(REMOVE_DUPLICATES _definitions)    
    #MESSAGE("_definitions: ${_definitions}")
    ADD_CUSTOM_COMMAND(
        OUTPUT ${_output}
        COMMAND ${CMAKE_CXX_COMPILER}
           ${_compiler_FLAGS}
           ${_include_FLAGS}
           ${_definitions}
           -x c++-header
           -o ${_output} ${_source}
        DEPENDS ${_source} )
    ADD_CUSTOM_TARGET(${_targetName} DEPENDS ${_output})
    #SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -include ${CMAKE_CURRENT_BINARY_DIR}/${_name} -Winvalid-pch -H")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -include ${CMAKE_CURRENT_BINARY_DIR}/${_name} -Winvalid-pch")
ENDMACRO(ADD_PRECOMPILED_HEADER)
