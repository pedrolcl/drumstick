/*
    Drumstick RT (realtime MIDI In/Out)
    Copyright (C) 2009-2015 Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

/**
 * @file backendmanager.h
 * Realtime MIDI input/output multiplatform classes
 * @defgroup RT Realtime MIDI (I/O)
 * @{
 */

namespace drumstick {
namespace rt {

    const QString QSTR_DRUMSTICK(QLatin1Literal("drumstick"));
    const QString QSTR_DRUMSTICKRT(QLatin1Literal("DRUMSTICKRT"));
    const QString QSTR_DRUMSTICKRT_GROUP(QLatin1Literal("DrumstickRT"));
    const QString QSTR_DRUMSTICKRT_PUBLICNAMEIN(QLatin1Literal("PublicNameIN"));
    const QString QSTR_DRUMSTICKRT_PUBLICNAMEOUT(QLatin1Literal("PublicNameOUT"));
    const QString QSTR_DRUMSTICKRT_EXCLUDED(QLatin1Literal("ExcludedNames"));
    const QString QSTR_DRUMSTICKRT_PATH(QLatin1Literal("BackendsPath"));

    /**
     * @brief The BackendManager class manages lists of dynamic and static
     * backends for applications based on drumstick-rt
     */
    class DRUMSTICK_EXPORT BackendManager
    {
    public:
        /**
         * @brief BackendManager constructor
         */
        explicit BackendManager();

        /**
         * @brief ~BackendManager destructor
         */
        virtual ~BackendManager();

        /**
         * @brief refresh the list of backends
         * @param settings Program settings
         */
        void refresh(QSettings* settings = 0);

        /**
         * @brief availableInputs
         * @return list of available MIDI inputs
         */
        QList<MIDIInput*> availableInputs();

        /**
         * @brief availableOutputs
         * @return list of available MIDI outputs
         */
        QList<MIDIOutput*> availableOutputs();

        /**
         * @brief defaultPaths
         * @return list of paths for backends search
         */
        QStringList defaultPaths();

    private:
        class BackendManagerPrivate;
        BackendManagerPrivate *d;
    };

}}

/** @} */

#endif // BACKENDMANAGER_H
