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

#include "maccommon.h"

#if QT_VERSION < QT_VERSION_CHECK(5,2,0)
    QString CFStringToQString(CFStringRef str)
    {
        if (!str)
            return QString();
        CFIndex length = CFStringGetLength(str);
        const UniChar *chars = CFStringGetCharactersPtr(str);
        if (chars)
            return QString(reinterpret_cast<const QChar *>(chars), length);
        QVarLengthArray<UniChar> buffer(length);
        CFStringGetCharacters(str, CFRangeMake(0, length), buffer.data());
        return QString(reinterpret_cast<const QChar *>(buffer.constData()), length);
    }
#endif

QString getEndpointName(MIDIEndpointRef endpoint)
{
    QString result;
    CFStringRef str = 0;
    MIDIObjectGetStringProperty (endpoint, kMIDIPropertyName, &str);
    if (str != 0) {
        result = QString::fromCFString(str);
        CFRelease(str);
        str = 0;
    }
    MIDIEntityRef entity = 0;
    MIDIEndpointGetEntity(endpoint, &entity);
    if (entity == 0)
        return result;
    if (result.isEmpty()) {
        MIDIObjectGetStringProperty (entity, kMIDIPropertyName, &str);
        if (str != 0) {
            result = QString::fromCFString(str);
            CFRelease(str);
            str = 0;
        }
    }
    MIDIDeviceRef device = 0;
    MIDIEntityGetDevice (entity, &device);
    if (device == 0)
        return result;
    MIDIObjectGetStringProperty (device, kMIDIPropertyName, &str);
    if (str != 0) {
        QString s = QString::fromCFString(str);
        CFRelease (str);
        str = 0;
        if (!result.startsWith(s, Qt::CaseInsensitive) )
            result = (s + ' ' + result).trimmed();
    }
    return result;
}

/* QString getEndpointName(MIDIEndpointRef endpoint)
{
    QString result;
    CFStringRef str = 0;
    MIDIObjectGetStringProperty (endpoint, kMIDIPropertyName, &str);
    if (str != 0) {
        result = QString::fromCFString(str);
        CFRelease(str);
        str = 0;
    }
    MIDIEntityRef entity = 0;
    MIDIEndpointGetEntity(endpoint, &entity);
    if (entity == 0)
        return result;
    if (result.isEmpty()) {
        MIDIObjectGetStringProperty (entity, kMIDIPropertyName, &str);
        if (str != 0) {
            result = QString::fromCFString(str);
            CFRelease(str);
            str = 0;
        }
    }
    MIDIDeviceRef device = 0;
    MIDIEntityGetDevice (entity, &device);
    if (device == 0)
        return result;
    MIDIObjectGetStringProperty (device, kMIDIPropertyName, &str);
    if (str != 0) {
        QString s =QString::fromCFString(str);
        CFRelease (str);
        str = 0;
        if (!result.startsWith(s, Qt::CaseInsensitive) )
            result = (s + ' ' + result).trimmed();
    }
    return result;
}
*/
