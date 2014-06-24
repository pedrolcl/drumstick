/*
    Drumstick RT (realtime MIDI In/Out)
    Copyright (C) 2009-2014 Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef MACCOMMON_H
#define MACCOMMON_H

#include <QString>
#include <CoreMIDI/CoreMIDI.h>

#if QT_VERSION < QT_VERSION_CHECK(5,2,0)
    QString CFStringToQString(CFStringRef str);
#endif

QString getEndpointName(MIDIEndpointRef endpoint);

#endif // MACCOMMON_H
