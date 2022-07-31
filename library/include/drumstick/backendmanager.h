/*
    Drumstick RT (realtime MIDI In/Out)
    Copyright (C) 2009-2022 Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#ifndef BACKENDMANAGER_H
#define BACKENDMANAGER_H

#include <QObject>
#include <QScopedPointer>
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
         */
        void refresh(QSettings* settings = nullptr);

        /**
         * @brief refresh the list of backends
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

        /**
         * @brief findInput returns the backend corresponding
         * to the provided name, or a suitable input instead.
         * @param name The name of some input backend
         * @return Input backend instance if available
         */
        MIDIInput* findInput(QString name);

        /**
         * @brief findOutput returns the backend corresponding
         * to the provided name, or a suitable output instead.
         * @param name The name of some output backend
         * @return Output backend instance if available
         */
        MIDIOutput* findOutput(QString name);

        static const QString QSTR_DRUMSTICK;
        static const QString QSTR_DRUMSTICK_VERSION;
        static const QString QSTR_DRUMSTICKRT;
        static const QString QSTR_DRUMSTICKRT_GROUP;
        static const QString QSTR_DRUMSTICKRT_PUBLICNAMEIN;
        static const QString QSTR_DRUMSTICKRT_PUBLICNAMEOUT;
        static const QString QSTR_DRUMSTICKRT_EXCLUDED;
        static const QString QSTR_DRUMSTICKRT_PATH;

    private:
        class BackendManagerPrivate;
        QScopedPointer<BackendManagerPrivate> d;
    };

    QString DRUMSTICK_EXPORT drumstickLibraryVersion();

/** @} */

}} // namespace drumstick::rt

#endif // BACKENDMANAGER_H
