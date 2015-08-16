/*
    Drumstick MIDI realtime input-output
    Copyright (C) 2009-2015 Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include <QDebug>
#include "midiparser.h"
#include "rtmidioutput.h"

namespace drumstick {
namespace rt {

class MIDIParser::MIDIParserPrivate {
public:
    MIDIParserPrivate(): m_in(0), m_out(0), m_running_status(0) { }
    MIDIInput *m_in;
    MIDIOutput *m_out;
    unsigned char m_running_status;
    QByteArray m_buffer;

    void processNoteOff(const int chan, const int note, const int vel)
    {
        //qDebug() << "NoteOff(" << hex << chan << "," << note << "," << vel << ")";
        if (m_in != 0 && m_in->isEnabledMIDIThru() && m_out != 0) {
            m_out->sendNoteOff(chan, note, vel);
        }
        if (m_in != 0) {
            m_in->emit midiNoteOff(chan, note, vel);
        }
    }

    void processNoteOn(const int chan, const int note, const int vel)
    {
        //qDebug() << "NoteOn(" << hex << chan << "," << note << "," << vel << ")";
        if (m_in != 0 && m_in->isEnabledMIDIThru() && m_out != 0) {
            m_out->sendNoteOn(chan, note, vel);
        }
        if (m_in != 0) {
            m_in->emit midiNoteOn(chan, note, vel);
        }
    }

    void processKeyPressure(const int chan, const int note, const int value)
    {
        //qDebug() << "KeyPressure(" << hex << chan << "," << note << "," << value << ")";
        if (m_in != 0 && m_in->isEnabledMIDIThru() && m_out != 0) {
            m_out->sendKeyPressure(chan, note, value);
        }
        if (m_in != 0) {
            m_in->emit midiKeyPressure(chan, note, value);
        }
    }

    void processController(const int chan, const int control, const int value)
    {
        //qDebug() << "Controller(" << chan << "," << control << "," << value << ")";
        if (m_in != 0 && m_in->isEnabledMIDIThru() && m_out != 0) {
            m_out->sendController(chan, control, value);
        }
        if (m_in != 0) {
            m_in->emit midiController(chan, control, value);
        }
    }

    void processProgram(const int chan, const int program)
    {
        //qDebug() << "Program(" << hex << chan << "," << program << ")";
        if (m_in != 0 && m_in->isEnabledMIDIThru() && m_out != 0) {
            m_out->sendProgram(chan, program);
        }
        if (m_in != 0) {
            m_in->emit midiProgram(chan, program);
        }
    }

    void processChannelPressure(const int chan, const int value)
    {
        //qDebug() << "ChannelPressure(" << chan << "," << value << ")";
        if (m_in != 0 && m_in->isEnabledMIDIThru() && m_out != 0) {
            m_out->sendChannelPressure(chan, value);
        }
        if (m_in != 0) {
            m_in->emit midiChannelPressure(chan, value);
        }
    }

    void processPitchBend(const int chan, const int value)
    {
        //qDebug() << "PitchBend(" << chan << "," << value << ")";
        if (m_in != 0 && m_in->isEnabledMIDIThru() && m_out != 0) {
            m_out->sendPitchBend(chan, value);
        }
        if (m_in != 0) {
            m_in->emit midiPitchBend(chan, value);
        }
    }

    void processSysex(const QByteArray &data)
    {
        //qDebug() << "Sysex(" << data.toHex() << ")";
        if (m_in != 0 && m_in->isEnabledMIDIThru() && m_out != 0) {
            m_out->sendSysex(data);
        }
        if (m_in != 0) {
            m_in->emit midiSysex(data);
        }
    }

    void processSystemCommon(const int status)
    {
        //qDebug() << "common SystemMsg(" << hex << status << ")";
        if (m_in != 0 && m_in->isEnabledMIDIThru() && m_out != 0) {
            m_out->sendSystemMsg(status);
        }
        if (m_in != 0) {
            m_in->emit midiSystemCommon(status);

        }
    }

    void processSystemRealtime(unsigned char byte)
    {
        //qDebug() << "realtime SystemMsg(" << hex << byte << ")";
        if (m_in != 0 && m_in->isEnabledMIDIThru() && m_out != 0) {
            m_out->sendSystemMsg(byte);
        }
        if (m_in != 0) {
            m_in->emit midiSystemRealtime(byte);
        }
    }

};

MIDIParser::MIDIParser(MIDIInput *in, QObject *parent) :
    QObject(parent),
    d(new MIDIParser::MIDIParserPrivate)
{
    d->m_buffer.clear();
    d->m_in = in;
}

MIDIParser::~MIDIParser()
{
    delete d;
}

void MIDIParser::setMIDIThruDevice(MIDIOutput *device)
{
    d->m_out = device;
}

void MIDIParser::parse(unsigned char byte)
{
    unsigned char status;
    int chan, m1, m2, v;

    if (byte >= MIDI_STATUS_REALTIME) { // system realtime
        d->processSystemRealtime(byte);
        return;
    } else
        d->m_buffer.append(byte);

    while(d->m_buffer.length() > 0) {
        status = static_cast<unsigned>(d->m_buffer.at(0));
        if (status == MIDI_STATUS_SYSEX) { // system exclusive
            if (byte == MIDI_STATUS_ENDSYSEX) {
                d->processSysex(d->m_buffer);
                d->m_buffer.clear();
            } else
                return;
        } else
        if (status > MIDI_STATUS_SYSEX &&
            status < MIDI_STATUS_ENDSYSEX) { // system common
            d->processSystemCommon(status);
            d->m_buffer.clear();
        } else
        if (status < MIDI_STATUS_SYSEX &&
            status >= MIDI_STATUS_NOTEOFF) { // channel message
            d->m_running_status = status;
            chan = status & MIDI_CHANNEL_MASK;
            status = status & MIDI_STATUS_MASK;
            switch(status) {
            case MIDI_STATUS_NOTEOFF:
                if (d->m_buffer.length() < 3)
                    return;
                m1 = static_cast<unsigned>(d->m_buffer.at(1));
                m2 = static_cast<unsigned>(d->m_buffer.at(2));
                d->processNoteOff(chan, m1, m2);
                break;
            case MIDI_STATUS_NOTEON:
                if (d->m_buffer.length() < 3)
                    return;
                m1 = static_cast<unsigned>(d->m_buffer.at(1));
                m2 = static_cast<unsigned>(d->m_buffer.at(2));
                d->processNoteOn(chan, m1, m2);
                break;
            case MIDI_STATUS_KEYPRESURE:
                if (d->m_buffer.length() < 3)
                    return;
                m1 = static_cast<unsigned>(d->m_buffer.at(1));
                m2 = static_cast<unsigned>(d->m_buffer.at(2));
                d->processKeyPressure(chan, m1, m2);
                break;
            case MIDI_STATUS_CONTROLCHANGE:
                if (d->m_buffer.length() < 3)
                    return;
                m1 = static_cast<unsigned>(d->m_buffer.at(1));
                m2 = static_cast<unsigned>(d->m_buffer.at(2));
                d->processController(chan, m1, m2);
                break;
            case MIDI_STATUS_PROGRAMCHANGE:
                if (d->m_buffer.length() < 2)
                    return;
                m1 = static_cast<unsigned>(d->m_buffer.at(1));
                d->processProgram(chan, m1);
                break;
            case MIDI_STATUS_CHANNELPRESSURE:
                if (d->m_buffer.length() < 2)
                    return;
                m1 = static_cast<unsigned>(d->m_buffer.at(1));
                d->processChannelPressure(chan, m1);
                break;
            case MIDI_STATUS_PITCHBEND:
                if (d->m_buffer.length() < 3)
                    return;
                m1 = static_cast<unsigned>(d->m_buffer.at(1));
                m2 = static_cast<unsigned>(d->m_buffer.at(2));
                v = m1 + m2 * 0x80 - 0x2000;
                d->processPitchBend(chan, v);
                break;
            }
            d->m_buffer.clear();
        } else { // running status
            d->m_buffer.insert(0, d->m_running_status);
        }
    }
}

void MIDIParser::parse(QByteArray bytes)
{
    foreach(unsigned char byte, bytes) {
        parse(byte);
    }
}

}}

