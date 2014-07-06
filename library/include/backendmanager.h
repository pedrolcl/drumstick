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

#ifndef BACKENDMANAGER_H
#define BACKENDMANAGER_H

#include <QObject>
#include "macros.h"
#include "rtmidiinput.h"
#include "rtmidioutput.h"

namespace drumstick {
namespace rt {

    const QString QSTR_DRUMSTICKRT_GROUP(QLatin1Literal("DrumstickRT"));
    const QString QSTR_DRUMSTICKRT_PUBLICNAMEIN(QLatin1Literal("PublicNameIN"));
    const QString QSTR_DRUMSTICKRT_PUBLICNAMEOUT(QLatin1Literal("PublicNameOUT"));
    const QString QSTR_DRUMSTICKRT_EXCLUDED(QLatin1Literal("ExcludedNames"));
    const QString QSTR_DRUMSTICKRT_PATH(QLatin1Literal("BackendsPath"));

    class DRUMSTICK_EXPORT BackendManager
    {
    public:
        explicit BackendManager();
        void refresh(QSettings* settings = 0);
        QList<MIDIInput*> inputsAvailable();
        QList<MIDIOutput*> outputsAvailable();
        QStringList defaultPaths();

    private:
        class BackendManagerPrivate;
        BackendManagerPrivate *d;
    };

}}

#endif // BACKENDMANAGER_H
