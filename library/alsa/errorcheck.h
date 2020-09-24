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
