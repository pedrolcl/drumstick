/*
    MIDI Sequencer C++ library
    Copyright (C) 2006-2020, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

extern "C" {
    #include <alsa/asoundlib.h>
}
#include <drumstick/sequencererror.h>

/**
 * @file sequencererror.cpp
 * SequencerError Exception class implementation
 */

namespace drumstick { namespace ALSA {

/**
* @addtogroup ALSAError
* @{
*/

SequencerError::SequencerError(QString const& s, int rc):
    m_location(s), m_errCode(rc) { }

const char *SequencerError::what() const noexcept
{
    return snd_strerror(m_errCode);
}

const QString SequencerError::qstrError() const
{
    return QString(what());
}

int SequencerError::code() const
{
    return m_errCode;
}

const QString &SequencerError::location() const
{
    return m_location;
}

/** @} */

}} // namespace drumstick::ALSA
