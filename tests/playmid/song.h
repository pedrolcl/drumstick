/*
    SMF GUI Player test using the MIDI Sequencer C++ library 
    Copyright (C) 2006-2008, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along 
    with this program; if not, write to the Free Software Foundation, Inc., 
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.    
*/

#ifndef INCLUDED_SONG_H
#define INCLUDED_SONG_H

#include <QStringList>

#include "event.h"

using namespace ALSA::Sequencer;

class Song : public QList<SequencerEvent>
{
public:
    enum TextType { 
        Text = 1, Copyright = 2, TrackName = 3,
        InstrumentName = 4, Lyric = 5, Marker = 6, Cue = 7
    };
    
    Song() : QList<SequencerEvent>(),
        m_format(0),
        m_ntrks(0),
        m_division(0)
    { }
    
    void clear();
    void sort();
    void setHeader(int format, int ntrks, int division);
    void setFileName(const QString& fileName);
    void addText(const int type, const QString& text);
    
    int getFormat() const { return m_format; }
    int getTracks() const { return m_ntrks; }
    int getDivision() const { return m_division; }
    QString getFileName() const { return m_fileName; }
    QString getText(const int type);
    QString getCopyright();

private:    
    int m_format;
    int m_ntrks;
    int m_division;    
    QString m_fileName;
    QStringList m_text;
};

typedef QListIterator<SequencerEvent> SongIterator;

#endif /*INCLUDED_SONG_H*/
