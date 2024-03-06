/*
    SMF GUI Player test using the MIDI Sequencer C++ library 
    Copyright (C) 2006-2024, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include "song.h"
#include <drumstick/alsaevent.h>

using namespace drumstick::ALSA;

static inline bool eventLessThan(const SequencerEvent* s1, const SequencerEvent* s2)
{
    return s1->getTick() < s2->getTick();
}

Song::~Song()
{
    clear();
}

void Song::sort() 
{
    std::sort(begin(), end(), eventLessThan);
}

void Song::clear()
{
    while (!isEmpty())
        delete takeFirst();
    m_fileName.clear();
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

void Song::setDivision(int division)
{
    m_division = division;
}

void Song::setFileName(const QString& fileName)
{
    m_fileName = fileName;
}
