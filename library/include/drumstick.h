/*
    Drumstick MIDI C++ library 
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

#ifndef DRUMSTICK_H
#define DRUMSTICK_H
#include <QtCore>

/**
 * @file drumstick.h
 * The main header that a program can include to use all drumstick features.
 */

#if defined(Q_OS_LINUX)
// ALSA library interface
#include <drumstick/alsaclient.h>
#include <drumstick/alsaevent.h>
#include <drumstick/alsaport.h>
#include <drumstick/alsaqueue.h>
#include <drumstick/alsatimer.h>
#include <drumstick/playthread.h>
#include <drumstick/subscription.h>
#include <drumstick/sequencererror.h>
#endif

// File formats
#include <drumstick/qsmf.h>
#include <drumstick/qwrk.h>
#include <drumstick/rmid.h>

// RealTime interfaces
#include <drumstick/rtmidiinput.h>
#include <drumstick/rtmidioutput.h>
#include <drumstick/backendmanager.h>

// Widgets
#include <drumstick/pianokeybd.h>
#include <drumstick/pianopalette.h>
#include <drumstick/settingsfactory.h>
#include <drumstick/configurationdialogs.h>

#endif /*DRUMSTICK_H*/
