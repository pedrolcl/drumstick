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

#include "player.h"
#include "song.h"
#include <cmath>
#include <drumstick/alsaclient.h>
#include <drumstick/alsaqueue.h>

using namespace drumstick::ALSA;

Player::Player(MidiClient *seq, int portId) 
    : SequencerOutputThread(seq, portId),
    m_song(nullptr),
    m_songIterator(nullptr),
    m_lastEvent(nullptr),
    m_songPosition(0),
    m_echoResolution(0),
    m_pitchShift(0),
    m_volumeFactor(100)
{
    for (int chan = 0; chan < MIDI_CHANNELS; ++chan)
        m_volume[chan] = 100;
}

Player::~Player()
{
    if (isRunning()) {
        stop();
    }
    delete m_songIterator;
    delete m_lastEvent;
}

void Player::setSong(Song* s)
{
    m_song = s;
    if (m_song != nullptr) {
        delete m_songIterator;
        m_songIterator = new SongIterator(*m_song);
        m_echoResolution = m_song->getDivision() / 12;
        m_songPosition = 0;
    }
}

void Player::resetPosition()
{
    if ((m_song != nullptr) && (m_songIterator != nullptr)) {
        m_songIterator->toFront();
        m_songPosition = 0;
    }
}

void Player::setPosition(unsigned int pos)
{
    m_songPosition = pos;
    m_songIterator->toFront();
    while (m_songIterator->hasNext() &&
          (m_songIterator->next()->getTick() < pos)) { };
    if (m_songIterator->hasPrevious())
        m_songIterator->previous();
}

bool Player::hasNext()
{
    auto res = m_songIterator->hasNext();
    return res;
}

SequencerEvent* Player::nextEvent()
{
    delete m_lastEvent;
    m_lastEvent = m_songIterator->next()->clone();
    switch (m_lastEvent->getSequencerType()) {
        case SND_SEQ_EVENT_NOTE:
        case SND_SEQ_EVENT_NOTEON:
        case SND_SEQ_EVENT_NOTEOFF:
        case SND_SEQ_EVENT_KEYPRESS: {
            KeyEvent* kev = static_cast<KeyEvent*>(m_lastEvent);
            if (kev->getChannel() != MIDI_GM_DRUM_CHANNEL)
                kev->setKey(kev->getKey() + m_pitchShift);
        }
        break;
        case SND_SEQ_EVENT_CONTROLLER: {
            ControllerEvent *cev = static_cast<ControllerEvent*>(m_lastEvent);
            if (cev->getParam() == MIDI_CTL_MSB_MAIN_VOLUME) {
                int chan = cev->getChannel();
                int value = cev->getValue();
                m_volume[chan] = value;
                value = floor(value * m_volumeFactor / 100.0);
                if (value < 0) value = 0;
                if (value > 127) value = 127;
                cev->setValue(value);
            }
        }
        break;
    }
    return m_lastEvent;
}

unsigned int Player::getInitialPosition()
{
    return m_songPosition;
}

unsigned int Player::getEchoResolution()
{
    return m_echoResolution;
}

unsigned int Player::getPitchShift()
{
    return m_pitchShift;
}

unsigned int Player::getVolumeFactor()
{
    return m_volumeFactor;
}

void Player::setPitchShift(unsigned int pitch)
{
    bool playing = isRunning();
    if (playing) {
        stop();
        unsigned int pos = m_Queue->getStatus().getTickTime();
        m_Queue->clear();
        allNotesOff();
        setPosition(pos);
    }
    m_pitchShift = pitch;
    if (playing)
        start();
}

void Player::setVolumeFactor(unsigned int vol)
{
    m_volumeFactor = vol;
    for(int chan = 0; chan < MIDI_CHANNELS; ++chan) {
        int value = m_volume[chan];
        value = floor(value * m_volumeFactor / 100.0);
        if (value < 0) value = 0;
        if (value > 127) value = 127;
        sendController(chan, MIDI_CTL_MSB_MAIN_VOLUME, value);
    }
}

void Player::sendController(int chan, int control, int value)
{
    ControllerEvent ev(chan, control, value);
    ev.setSource(m_PortId);
    ev.setSubscribers();
    ev.setDirect();
    sendSongEvent(&ev);
}

void Player::allNotesOff()
{
    for(int chan = 0; chan < MIDI_CHANNELS; ++chan) {
        sendController(chan, MIDI_CTL_ALL_NOTES_OFF, 0);
        sendController(chan, MIDI_CTL_ALL_SOUNDS_OFF, 0);
    }
}

void Player::sendVolumeEvents()
{
    for(int chan = 0; chan < MIDI_CHANNELS; ++chan) {
        int value = m_volume[chan] = 100;
        value = floor(value * m_volumeFactor / 100.0);
        if (value < 0) value = 0;
        if (value > 127) value = 127;
        sendController(chan, MIDI_CTL_MSB_MAIN_VOLUME, value);
    }
}
