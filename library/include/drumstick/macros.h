/*
    MIDI Sequencer C++ library
    Copyright (C) 2006-2021, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DRUMSTICK_MACROS_H
#define DRUMSTICK_MACROS_H

#include <qglobal.h>

/**
 * @file macros.h
 * Drumstick visibility macros
 */

#if !defined(DRUMSTICK_EXPORT) 
#  if defined(DRUMSTICK_STATIC)
#    define DRUMSTICK_EXPORT
#  else
#    if defined(drumstick_alsa_EXPORTS) || defined(drumstick_file_EXPORTS) || defined(drumstick_rt_EXPORTS) || defined(drumstick_widgets_EXPORTS)
#      define DRUMSTICK_EXPORT Q_DECL_EXPORT
#    else
#      if defined(Q_OS_WIN)
#        define DRUMSTICK_EXPORT Q_DECL_IMPORT
#      else
#        define DRUMSTICK_EXPORT Q_DECL_EXPORT
#      endif
#    endif
#  endif
#endif

#if defined(_MSC_VER)
    #define DISABLE_WARNING_PUSH           __pragma(warning( push ))
    #define DISABLE_WARNING_POP            __pragma(warning( pop ))
    #define DISABLE_WARNING(warningNumber) __pragma(warning( disable : warningNumber ))

    #define DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER    DISABLE_WARNING(4100)
    #define DISABLE_WARNING_UNREFERENCED_FUNCTION            DISABLE_WARNING(4505)
    #define DISABLE_WARNING_DEPRECATED_DECLARATIONS          DISABLE_WARNING(4996)

#elif defined(__GNUC__) || defined(__clang__)
    #define DO_PRAGMA(X) _Pragma(#X)
    #define DISABLE_WARNING_PUSH           DO_PRAGMA(GCC diagnostic push)
    #define DISABLE_WARNING_POP            DO_PRAGMA(GCC diagnostic pop)
    #define DISABLE_WARNING(warningName)   DO_PRAGMA(GCC diagnostic ignored #warningName)

    #define DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER    DISABLE_WARNING(-Wunused-parameter)
    #define DISABLE_WARNING_UNREFERENCED_FUNCTION            DISABLE_WARNING(-Wunused-function)
    #define DISABLE_WARNING_DEPRECATED_DECLARATIONS          DISABLE_WARNING(-Wdeprecated-declarations)

#else
    #define DISABLE_WARNING_PUSH
    #define DISABLE_WARNING_POP
    #define DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER
    #define DISABLE_WARNING_UNREFERENCED_FUNCTION
    #define DISABLE_WARNING_DEPRECATED_DECLARATIONS
#endif

#endif /* DRUMSTICK_MACROS_H */
