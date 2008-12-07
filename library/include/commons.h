/*
    MIDI Sequencer C++ library 
    Copyright (C) 2006-2008, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#ifndef COMMONS_H_
#define COMMONS_H_

#include <qglobal.h>
#include <QString>
#include <QApplication>
#include <QtDebug>

extern "C" {
#include <alsa/asoundlib.h>
}

namespace ALSA 
{
namespace Sequencer 
{

typedef quint8 MidiByte;  

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

}

inline int checkErrorAndThrow(int rc, const char *where)
{
    if (rc < 0) {
        qDebug() << "Error code:" << rc << "(" <<  snd_strerror(rc) << ")";
        qDebug() << "Location:" << where;
        throw Sequencer::SequencerError(QString(where), rc);
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

}

#endif /*COMMONS_H_*/
