/*
    Drumstick RT (realtime MIDI In/Out)
    Copyright (C) 2009-2024 Pedro Lopez-Cabanillas <plcl@users.sf.net>

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
*/

#ifndef MACCOMMON_H
#define MACCOMMON_H

#include <QString>
#include <CoreMIDI/CoreMIDI.h>

#if QT_VERSION < QT_VERSION_CHECK(5,2,0)
    QString CFStringToQString(CFStringRef str);
#endif

QString getEndpointName(MIDIEndpointRef endpoint);

QString getErrorTextFromOSStatus(OSStatus err);

#endif // MACCOMMON_H
