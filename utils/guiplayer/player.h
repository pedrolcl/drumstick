/*
    SMF GUI Player test using the MIDI Sequencer C++ library 
    Copyright (C) 2006-2019, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef INCLUDED_PLAYER_H
#define INCLUDED_PLAYER_H

#include <drumstick/playthread.h>
#include "song.h"

class Player : public drumstick::ALSA::SequencerOutputThread
{
    Q_OBJECT
    
public:
    Player(drumstick::ALSA::MidiClient *seq, int portId);
	virtual ~Player();
    virtual bool hasNext();
    virtual drumstick::ALSA::SequencerEvent* nextEvent();
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
    drumstick::ALSA::SequencerEvent* m_lastEvent;
    unsigned int m_songPosition;
    unsigned int m_echoResolution;
    unsigned int m_pitchShift;
    unsigned int m_volumeFactor;
    int m_volume[MIDI_CHANNELS];
};

#endif /*INCLUDED_PLAYER_H*/
