/*
    Drumstick RT Backend
    Copyright (C) 2009-2010 Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include "macmidiinput.h"
#include "macmidioutput.h"

#include <QDebug>
#include <QStringList>
#include <QMutex>
#include <QTextCodec>
#include <QObject>

//#include <CoreFoundation/CoreFoundation.h>
//#include <CoreServices/CoreServices.h>
//#include <AudioToolbox/AudioToolbox.h>
//#include <AudioToolbox/MusicPlayer.h>
#include <CoreMIDI/CoreMIDI.h>
#include <CoreMIDI/MIDIServices.h>

namespace drumstick {
namespace rt {

    void MacMIDIReadProc( const MIDIPacketList *pktlist,
                          void *refCon, void *connRefCon )
    {
        MacMIDIInput::MacMIDIInputPrivate  *obj = NULL;
        if (refCon != NULL)
            obj = static_cast<MacMIDIInput::MacMIDIInputPrivate*>(refCon);
        if (obj != NULL)
            obj->sendEvents(pktlist);
        MIDIPacket *packet = (MIDIPacket *)pktlist->packet;
        for (unsigned int i = 0; i < pktlist->numPackets; ++i) {
            if (obj != NULL)
               obj->emitSignals(packet);
            packet = MIDIPacketNext(packet);
        }
    }

    class MacMIDIInput::MacMIDIInputPrivate {
    public:
        MIDIEndpointRef m_endpoint;
        MIDIOutput *m_out;
        QMutex m_openMutex;


        MacMIDIInputPrivate(MIDIInput *inp) :
            m_player(0),
            m_sequence(0),
            m_endpoint(0),
            m_output(0),
            m_codec(0),
            m_state(BufferingState),
            m_playlistIndex(-1),
            m_tempoFactor(1.0),
            m_lowestMidiNote(127),
            m_highestMidiNote(0),
            m_sequenceLength(0),
            m_duration(0),
            m_division(0),
            m_songEmpty(true),
            m_tickInterval(0),
            m_numtracks(0),
            m_barCount(0),
            m_lyricTextType(Text)
        {
        }

        virtual ~MacMIDIInputPrivate()
        {
            OSStatus result;
            result = DisposeMusicPlayer(m_player);
            if (result != noErr)
                qDebug() << "DisposeMusicPlayer() err:" << result;
            if (m_sequence != NULL) {
                result = DisposeMusicSequence(m_sequence);
                if (result != noErr)
                    qDebug() << "DisposeMusicSequence() err:" << result;
            }
            if (m_endpoint != NULL) {
                result = MIDIEndpointDispose(m_endpoint);
                if (result != noErr)
                    qDebug() << "MIDIEndpointDispose() err:" << result;
            }
        }

        void initialize(MacMIDIOutput *output)
        {
            OSStatus result;
            d->m_output = output;
            result = MIDIDestinationCreate ( output->client(),
                        CFSTR("Drumstick Karaoke Loopback"), MacMIDIReadProc,
                        (void*) this, &d->m_endpoint );
            if (result != noErr)
                qDebug() << "MIDIDestinationCreate() err:" << result;
            else {
                result = MIDIObjectSetIntegerProperty( d->m_endpoint,
                            kMIDIPropertyPrivate, 1 );
                if (result != noErr)
                    qDebug() << "MIDIObjectSetIntegerProperty() err:" << result;
            }
        }

        void emitSignals(MIDIPacket* packet)
        {
            int value = 0;
            int status = packet->data[0] & 0xf0;
            int channel = packet->data[0] & 0x0f;
            QByteArray data;
            switch (status) {
            case MIDI_STATUS_NOTEOFF:
                emit midiNoteOff(channel, packet->data[1], packet->data[2]);
                break;
            case MIDI_STATUS_NOTEON:
                emit midiNoteOn(channel, packet->data[1], packet->data[2]);
                break;
            case MIDI_STATUS_KEYPRESURE:
                emit midiKeyPressure(channel, packet->data[1], packet->data[2]);
                break;
            case MIDI_STATUS_CONTROLCHANGE:
                emit midiController(channel, packet->data[1], packet->data[2]);
                break;
            case MIDI_STATUS_PROGRAMCHANGE:
                emit midiProgram(channel, packet->data[1]);
                break;
            case MIDI_STATUS_CHANNELPRESSURE:
                emit midiChannelPressure(channel, packet->data[1]);
                break;
            case MIDI_STATUS_PITCHBEND:
                value = (packet->data[1] + packet->data[2] * 0x80) - 8192;
                emit midiPitchBend(channel, value);
                break;
            case MIDI_STATUS_SYSEX:
                data = QByteArray((const char *)packet->data, packet->length);
                emit midiSysex(data);
                break;
            default:
                qDebug() << "status?" << status;
            }
        }

        void sendEvents(const MIDIPacketList *pktlist)
        {
            MacMIDIOutput *out = d->m_output;
            if (out != NULL)
               out->sendEvents(pktlist);
        }
    };

    MacMIDIInput::MacMIDIInput(QObject *parent) :
            MIDIInput(parent), d(new MacMIDIObjectPrivate(this))
    {
    }

    MacMIDIObject::~MacMIDIObject()
    {
        delete d;
    }

    /* SLOTS */



}} // namespace drumstick::rt

//#include "macmidiobject.moc"
