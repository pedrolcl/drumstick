/*
    MIDI Sequencer C++ library 
    Copyright (C) 2006-2010, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along 
    with this program; if not, write to the Free Software Foundation, Inc., 
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.    
*/

#ifndef DRUMSTICK_DRUMSTICKCOMMON_H
#define DRUMSTICK_DRUMSTICKCOMMON_H

#include "macros.h"
#include <qglobal.h>
#include <QString>
#include <QApplication>
#include <QtDebug>

extern "C" {
#include <alsa/asoundlib.h>
}

/**
 * @file drumstickcommon.h
 * Common functionality
 * @defgroup Common Common Functionality
 * @{
 * drumstick::SequencerError is a common exception object, encapsulating ALSA error codes.
 */

namespace drumstick {

/**
 * 8-bit unsigned number to be used as a MIDI message parameter
 */
typedef quint8 MidiByte;  

/**
 * Class used to report errors from the ALSA sequencer.
 *
 * The class SequencerError represents an exception object reported when the
 * ALSA library returns an error code. It is only used for severe errors.
 */
class DRUMSTICK_EXPORT SequencerError
{
public:
    /**
     * Constructor
     * @param s  Error location
     * @param rc Numeric error code
     */
    SequencerError(QString const& s, int rc) :
        m_location(s), m_errCode(rc) {}

    /**
     * Destructor
     */
    virtual ~SequencerError() {}

    /**
     * Gets the human readable error message from the error code
     * @return Error message
     */
    const QString qstrError() const 
    {
        return QString(snd_strerror(m_errCode));
    }

    /**
     * Gets the numeric error code
     * @return Error code
     */
    int code() const 
    { 
        return m_errCode; 
    }
    
    /**
     * Gets the location of the error code as provided in the constructor
     * @return Error location
     */
    const QString& location() const 
    { 
        return m_location; 
    }

private:
    QString m_location;
    int     m_errCode;
};

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
#define CHECK_ERROR(x)   (checkErrorAndThrow((x),__PRETTY_FUNCTION__))

/**
 * This macro calls the check warning function.
 * @param x Error code
 */
#define CHECK_WARNING(x) (checkWarning((x),__PRETTY_FUNCTION__))

/**
 * ALSA library version as a constant string.
 *
 * This string corresponds to the compilation library, which may be
 * different to the runtime library.
 * @see getRuntimeALSALibraryVersion
 */
const QString LIBRARY_VERSION(SND_LIB_VERSION_STR);

} /* namespace drumstick */

/** @} */

#endif /*DRUMSTICK_DRUMSTICKCOMMON_H*/
