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

#ifndef SEQUENCERERROR_H
#define SEQUENCERERROR_H

#include <QString>
#include "macros.h"
#include <exception>

/**
 * @file sequencererror.h
 * SequencerError Exception class
 */

/**
 * @namespace std
 * C++ Standard Library namespace
 *
 * @class std::exception
 * Provides consistent interface to handle errors.
 * @see https://en.cppreference.com/w/cpp/error/exception
 */

#if defined(DRUMSTICK_STATIC)
#define DRUMSTICK_ALSA_EXPORT
#else
#if defined(drumstick_alsa_EXPORTS)
#define DRUMSTICK_ALSA_EXPORT Q_DECL_EXPORT
#else
#define DRUMSTICK_ALSA_EXPORT Q_DECL_IMPORT
#endif
#endif

namespace drumstick {
namespace ALSA {

/**
 * @addtogroup ALSAError ALSA Sequencer Exception
 * @{
 *
 * @class SequencerError
 * Exception class for ALSA Sequencer errors.
 * This class is used to report errors from the ALSA sequencer.
 *
 * The class SequencerError represents an exception object reported when the
 * ALSA library returns an error code. It is only used for severe errors.
 */
class DRUMSTICK_ALSA_EXPORT SequencerError : std::exception
{
public:
    /**
     * Constructor
     * @param s  Error location
     * @param rc Numeric error code
     */
    SequencerError(QString const& s, int rc);

    /**
     * Retrieve a human readable error message
     * @return human readable error message
     */
    virtual const char* what() const noexcept override;

    /**
     * Gets the human readable error message from the error code
     * @return Error message
     */
    QString qstrError() const;

    /**
     * Gets the numeric error code
     * @return Error code
     */
    int code() const;

    /**
     * Gets the location of the error code as provided in the constructor
     * @return Error location
     */
    const QString& location() const;

private:
    QString m_location;
    int     m_errCode;
};

/** @} */

} // namespace ALSA
} // namespace drumstick

#endif // SEQUENCERERROR_H
