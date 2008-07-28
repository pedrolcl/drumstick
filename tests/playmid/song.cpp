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

#include "song.h"

static inline bool eventLessThan(const SequencerEvent &s1, const SequencerEvent &s2)
{
    return s1.getTick() < s2.getTick();
}

void Song::sort() 
{
    qStableSort(begin(), end(), eventLessThan);
}

void Song::clear()
{
    QList<SequencerEvent>::clear();
    m_fileName.clear();
    m_text.clear();
    m_format = 0;
    m_ntrks = 0;
    m_division = 0;
}

void Song::setHeader(int format, int ntrks, int division)
{
    m_format = format;
    m_ntrks = ntrks;
    m_division = division;
}

void Song::setFileName(const QString& fileName)
{
    m_fileName = fileName;
}

void Song::addText(const int type, const QString& text)
{
    while (type >= m_text.size()) {
        m_text.append(QString());
    }
    m_text[type] += text;
}

QString Song::getText(const int type) 
{
    return m_text.value(type);
}

QString Song::getCopyright() 
{
    return getText(Copyright);
}
