#ifndef MACCOMMON_H
#define MACCOMMON_H

#include <QString>
#include <CoreMIDI/CoreMIDI.h>

#if QT_VERSION < QT_VERSION_CHECK(5,2,0)
    QString CFStringToQString(CFStringRef str);
#endif

QString getEndpointName(MIDIEndpointRef endpoint);

#endif // MACCOMMON_H
