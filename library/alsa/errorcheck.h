/*
    MIDI Sequencer C++ library
    Copyright (C) 2006-2023, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#ifndef ERRORCHECK_H
#define ERRORCHECK_H

extern "C" {
#include <alsa/asoundlib.h>
}

#include <QString>
#include <QDebug>
#include <drumstick/sequencererror.h>

/**
 * @file errorcheck.h
 * Error checking functions and macros
 */

namespace drumstick { namespace ALSA {

/**
 * @addtogroup ALSAError
 * @{
 */

/**
 * Checks the error code for severe errors.
 * If the provided error code is less than zero an exception is thrown,
 * containing both the error code and the location.
 * @param rc Error code
 * @param where Location
 * @return Error code
 */
inline int checkErrorAndThrow(int rc, const char *where)
{
    if (rc < 0) {
        qDebug() << "Error code:" << rc << "(" <<  snd_strerror(rc) << ")";
        qDebug() << "Location:" << where;
        throw SequencerError(QString(where), rc);
    }
    return rc;
}

/**
 * Check the error code for warning errors.
 * This method doesn't throw an exception.
 * @param rc Error code
 * @param where Location
 * @return Error code
 */
inline int checkWarning(int rc, const char *where)
{
    if (rc < 0) {
        qWarning() << "Exception code:" << rc << "(" <<  snd_strerror(rc) << ")";
        qWarning() << "Location:" << where;
    }
    return rc;
}

/**
 * This macro calls the check error function.
 * @param x Error code
 */
#define DRUMSTICK_ALSA_CHECK_ERROR(x)   (checkErrorAndThrow((x),__PRETTY_FUNCTION__))

/**
 * This macro calls the check warning function.
 * @param x Error code
 */
#define DRUMSTICK_ALSA_CHECK_WARNING(x) (checkWarning((x),__PRETTY_FUNCTION__))

/** @} */

}} // namespace drumstick::ALSA

#endif // ERRORCHECK_H
