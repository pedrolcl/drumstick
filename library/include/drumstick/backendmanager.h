/*
    Drumstick RT (realtime MIDI In/Out)
    Copyright (C) 2009-2020 Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef BACKENDMANAGER_H
#define BACKENDMANAGER_H

#include <QObject>
#include "macros.h"
#include "rtmidiinput.h"
#include "rtmidioutput.h"

/**
 * @file backendmanager.h
 * BackendManager class declaration
 */

namespace drumstick {
/**
 * @ingroup RT
 * @brief Drumstick Real-Time library
 */
namespace rt {

    /**
     * @addtogroup RT Realtime MIDI (I/O)
     * @{
     */

    const QString QSTR_DRUMSTICK = QLatin1String("drumstick2");
    const QString QSTR_DRUMSTICKRT = QLatin1String("DRUMSTICKRT");
    const QString QSTR_DRUMSTICKRT_GROUP = QLatin1String("DrumstickRT");
    const QString QSTR_DRUMSTICKRT_PUBLICNAMEIN = QLatin1String("PublicNameIN");
    const QString QSTR_DRUMSTICKRT_PUBLICNAMEOUT = QLatin1String("PublicNameOUT");
    const QString QSTR_DRUMSTICKRT_EXCLUDED = QLatin1String("ExcludedNames");
    const QString QSTR_DRUMSTICKRT_PATH = QLatin1String("BackendsPath");

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
        void refresh(QSettings* settings = nullptr);

        /**
         * @brief refresh the list of backends
         * @param map Program settings relevant section
         */
        void refresh(const QVariantMap& map);

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

        /**
         * @brief inputBackendByName
         * @param name The name of some input backend
         * @return Input backend instance if available
         */
        MIDIInput* inputBackendByName(const QString name);

        /**
         * @brief outputBackendByName
         * @param name The name of some output backend
         * @return Output backend instance if available
         */
        MIDIOutput* outputBackendByName(const QString name);

    private:
        class BackendManagerPrivate;
        BackendManagerPrivate *d;
    };

/** @} */

}} // namespace drumstick::rt

#endif // BACKENDMANAGER_H
