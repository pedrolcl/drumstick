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
