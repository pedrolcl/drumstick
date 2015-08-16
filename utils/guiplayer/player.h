/*
    SMF GUI Player test using the MIDI Sequencer C++ library 
    Copyright (C) 2006-2015, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#ifndef INCLUDED_PLAYER_H
#define INCLUDED_PLAYER_H

#include "playthread.h" 
#include "song.h"

using namespace drumstick;

class Player : public SequencerOutputThread
{
    Q_OBJECT
    
public:
	Player(MidiClient *seq, int portId);
	virtual ~Player();
    virtual bool hasNext();
    virtual SequencerEvent* nextEvent();
    virtual unsigned int getInitialPosition() { return m_songPosition; }
    virtual unsigned int getEchoResolution() { return m_echoResolution; }
    unsigned int getPitchShift() { return m_pitchShift; }
    unsigned int getVolumeFactor() { return m_volumeFactor; }
    void setSong(Song* s);
    void resetPosition();
    void setPosition(unsigned int pos);
    void setPitchShift(unsigned int pitch);
    void setVolumeFactor(unsigned int vol);
    void sendController(int chan, int control, int value);
    void allNotesOff();
    void sendVolumeEvents();

private:
    Song* m_song;
    SongIterator* m_songIterator;
    SequencerEvent* m_lastEvent;
    unsigned int m_songPosition;
    unsigned int m_echoResolution;
    unsigned int m_pitchShift;
    unsigned int m_volumeFactor;
    int m_volume[MIDI_CHANNELS];
};

#endif /*INCLUDED_PLAYER_H*/
