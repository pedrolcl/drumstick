/*
    MIDI Sequencer C++ library
    Copyright (C) 2006-2010, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef DRUMSTICK_MACROS_H
#define DRUMSTICK_MACROS_H

#include <qglobal.h>

/**
 * @file macros.h
 * Drumstick visibility macros
 * @defgroup Common Common Functionality
 * @{
 */

#if !defined(DRUMSTICK_EXPORT) 
#  if defined(DRUMSTICK_STATIC)
#    define DRUMSTICK_EXPORT
#  else
#    if defined(drumstick_alsa_EXPORTS) || defined(drumstick_file_EXPORTS)
#      define DRUMSTICK_EXPORT Q_DECL_EXPORT
#    else
#      if defined(Q_WS_WIN)
#        define DRUMSTICK_EXPORT Q_DECL_IMPORT
#      else
#        define DRUMSTICK_EXPORT Q_DECL_EXPORT
#      endif
#    endif
#  endif
#endif

/** @} */

#endif /* DRUMSTICK_MACROS_H */
