/*
    MIDI Sequencer C++ library 
    Copyright (C) 2006-2009, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#ifndef ASEQMMCOMMON_H_
#define ASEQMMCOMMON_H_

/*!
 * @file aseqmmcommon.h
 * Common functionality
 * @defgroup Common Common functionality
 * @{
 */

#include <qglobal.h>
#include <QString>
#include <QApplication>
#include <QtDebug>

extern "C" {
#include <alsa/asoundlib.h>
}

#if defined(ASEQMM_NAMESPACES)
#define BEGIN_ALSA_NAMESPACE     namespace ALSA {
#define END_ALSA_NAMESPACE       }
#define BEGIN_ALSASEQ_NAMESPACE  namespace ALSA { namespace Sequencer {
#define END_ALSASEQ_NAMESPACE    } }
#define BEGIN_MIDI_NAMESPACE     namespace MIDI {
#define END_MIDI_NAMESPACE       }
#define BEGIN_MIDIUTIL_NAMESPACE namespace MIDI { namespace Utils {
#define END_MIDIUTIL_NAMESPACE   } }
#define USE_ALSA_NAMESPACE       using namespace ALSA;
#define USE_ALSASEQ_NAMESPACE    using namespace ALSA::Sequencer;
#define USE_MIDI_NAMESPACE       using namespace MIDI;
#define USE_MIDIUTIL_NAMESPACE   using namespace MIDI::Utils;
#else
#define BEGIN_ALSA_NAMESPACE
#define END_ALSA_NAMESPACE
#define BEGIN_ALSASEQ_NAMESPACE
#define END_ALSASEQ_NAMESPACE
#define BEGIN_MIDI_NAMESPACE
#define END_MIDI_NAMESPACE
#define BEGIN_MIDIUTIL_NAMESPACE
#define END_MIDIUTIL_NAMESPACE
#define USE_ALSA_NAMESPACE
#define USE_ALSASEQ_NAMESPACE
#define USE_MIDI_NAMESPACE
#define USE_MIDIUTIL_NAMESPACE
#endif

BEGIN_ALSA_NAMESPACE

typedef quint8 MidiByte;  

/**
 * Class used to report errors from the ALSA sequencer
 */
class SequencerError
{
public:
    SequencerError(QString const& s, int rc) :
        m_location(s), m_errCode(rc) {}
    
    virtual ~SequencerError() {}
    
    const QString qstrError() const 
    {
        return QString(snd_strerror(m_errCode));
    }

    int code() const 
    { 
        return m_errCode; 
    }
    
    const QString& location() const 
    { 
        return m_location; 
    }

private:
    QString m_location;
    int     m_errCode;
};

inline int checkErrorAndThrow(int rc, const char *where)
{
    if (rc < 0) {
        qDebug() << "Error code:" << rc << "(" <<  snd_strerror(rc) << ")";
        qDebug() << "Location:" << where;
        throw SequencerError(QString(where), rc);
    }
    return rc;
}

inline int checkWarning(int rc, const char *where)
{
    if (rc < 0) {
        qWarning() << "Exception code:" << rc << "(" <<  snd_strerror(rc) << ")";
        qWarning() << "Location:" << where;
    }
    return rc;
}

#define CHECK_ERROR(x)   (checkErrorAndThrow((x),__PRETTY_FUNCTION__))
#define CHECK_WARNING(x) (checkWarning((x),__PRETTY_FUNCTION__))

const QString LIBRARY_VERSION(SND_LIB_VERSION_STR);

END_ALSA_NAMESPACE

/*! @} */

#endif /*ASEQMMCOMMON_H_*/
