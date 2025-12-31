/*
    SMF GUI Player test using the MIDI Sequencer C++ library 
    Copyright (C) 2006-2025, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#ifndef INCLUDED_SONG_H
#define INCLUDED_SONG_H

#include <QStringList>

namespace drumstick { namespace ALSA {
    class SequencerEvent;
}}

class Song : public QList<drumstick::ALSA::SequencerEvent*>
{
public:
    Song() : QList<drumstick::ALSA::SequencerEvent*>(),
        m_format(0),
        m_ntrks(0),
        m_division(0)
    { }
    virtual ~Song();
    
    void clear();
    void sort();
    void setHeader(int format, int ntrks, int division);
    void setDivision(int division);
    void setFileName(const QString& fileName);
    
    int getFormat() const { return m_format; }
    int getTracks() const { return m_ntrks; }
    int getDivision() const { return m_division; }
    QString getFileName() const { return m_fileName; }

private:    
    int m_format;
    int m_ntrks;
    int m_division;    
    QString m_fileName;
};

typedef QListIterator<drumstick::ALSA::SequencerEvent*> SongIterator;

#endif /*INCLUDED_SONG_H*/
