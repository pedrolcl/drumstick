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

find_package(Subversion QUIET)
if (Subversion_FOUND)
    Subversion_WC_INFO(${PROJECT_SOURCE_DIR} PROJECT IGNORE_SVN_FAILURE)
    if (DEFINED PROJECT_WC_REVISION)
        message(STATUS "Current revision (SVN) is ${PROJECT_WC_REVISION}")
    endif()
endif()

if (NOT DEFINED PROJECT_WC_REVISION)
    find_package(Git QUIET)
    if (Git_FOUND)
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            RESULT_VARIABLE res
            OUTPUT_VARIABLE PROJECT_WC_REVISION
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE)
        if (${res} EQUAL 0)
            message(STATUS "Current revision (Git) is ${PROJECT_WC_REVISION}")
        else()
            unset(PROJECT_WC_REVISION)
        endif()
    endif()
endif()

if (DEFINED PROJECT_WC_REVISION)
    set(${PROJECT_NAME}_WC_REVISION ${PROJECT_WC_REVISION})
endif()
