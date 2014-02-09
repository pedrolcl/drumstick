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

#include "macmidioutput.h"

#include <QObject>
#include <QDebug>
#include <QStringList>
#include <QByteArray>
#include <QVarLengthArray>
#include <CoreMIDI/MIDIServices.h>

namespace drumstick {
namespace rt {

    class MacMIDIOutput::MacMIDIOutputPrivate {
    public:
        MacMIDIOutputPrivate::MacMIDIOutputPrivate():
            m_client(NULL),
            m_outPort(NULL),
            m_destination(NULL),
            m_pitchShift(0),
            m_clientFilter(true)
        {
        }

        virtual MacMIDIOutputPrivate::~MacMIDIOutputPrivate()
        {
            OSStatus result;
            if (m_client != NULL)
              result = MIDIClientDispose(m_client);
            if (result != noErr)
                qDebug() << "MIDIClientDispose() error:" << result;
        }


        void transform(MIDIPacket *packet)
        {
            quint8 status = packet->data[0] & 0xf0;
            //quint8 chan = packet->data[0] & 0x0f;
            switch ( status ) {
            case MIDI_STATUS_CONTROLCHANGE:
                transformControllerEvent(packet);
                break;
            case MIDI_STATUS_NOTEOFF:
            case MIDI_STATUS_NOTEON:
                transformNoteEvent(packet);
                break;
            case MIDI_STATUS_PROGRAMCHANGE:
                transformProgramEvent(packet);
                break;
            /*case MIDI_STATUS_PITCHBEND:
                transformPitchBendEvent(packet);
                break;*/
            default:
                break;
            }
            /*if ( status >= MIDI_STATUS_NOTEOFF &&
                 status <  MIDI_STATUS_SYSEX &&
                 m_mapper != NULL && m_mapper->isOK() ) {
                int channel = m_mapper->channel(chan);
                if (channel >= 0 && channel < MIDI_CHANNELS)
                    packet->data[0] = status + channel;
            }*/
        }

        MIDIClientRef m_client;
        MIDIPortRef m_outPort;
        MIDIEndpointRef m_destination;
        //MidiMapper *m_mapper;
        int m_pitchShift;
        bool m_clientFilter;
        QString m_currentOutput;
        QStringList m_outputDevices;

        int m_lastpgm[MIDI_CHANNELS];
        qreal m_volumeShift[MIDI_CHANNELS];
        int m_volume[MIDI_CHANNELS];
        bool m_muted[MIDI_CHANNELS];
        bool m_locked[MIDI_CHANNELS];
        int m_lockedpgm[MIDI_CHANNELS];
        QByteArray m_resetMessage;
    };

    MacMIDIOutput::MacMIDIOutput(QObject *parent) :
        MIDIOutput(parent), d(new MacMIDIOutputPrivate)
    {
        OSStatus result;
        result = MIDIClientCreate(CFSTR("Drumstick Karaoke"), NULL, NULL, &d->m_client);
        if ( result != noErr )
            return;
        result = MIDIOutputPortCreate(d->m_client, CFSTR("Drumstick Karaoke Output"), &d->m_outPort);
        if ( result != noErr )
            return;
        reloadDeviceList();
    }

    MacMIDIOutput::~MacMIDIOutput()
    {
        delete d;
    }

    QString CFStringToQString(CFStringRef str)
    {
        if (!str)
            return QString();
        CFIndex length = CFStringGetLength(str);
        const UniChar *chars = CFStringGetCharactersPtr(str);
        if (chars)
            return QString(reinterpret_cast<const QChar *>(chars), length);
        QVarLengthArray<UniChar> buffer(length);
        CFStringGetCharacters(str, CFRangeMake(0, length), buffer.data());
        return QString(reinterpret_cast<const QChar *>(buffer.constData()), length);
    }


    int MacMIDIOutput::outputDevice() const
    {
        return d->m_outputDevices.indexOf(d->m_currentOutput);
    }

    QString MacMIDIOutput::outputDeviceName() const
    {
        return d->m_currentOutput;
    }

    bool MacMIDIOutput::isMuted(int channel) const
    {
        if (channel >= 0 && channel < MIDI_CHANNELS)
            return d->m_muted[channel];
        return false;
    }

    int MacMIDIOutput::pitchShift()
    {
        return d->m_pitchShift;
    }

    QStringList MacMIDIOutput::outputDeviceList( bool basicOnly )
    {
        d->m_clientFilter = basicOnly;
        reloadDeviceList();
        return d->m_outputDevices;
    }

    /* SLOTS */

    void MacMIDIOutput::setVolume(int channel, qreal value)
    {
        if (channel >= 0 && channel < MIDI_CHANNELS) {
            d->m_volumeShift[channel] = value;
            sendController(channel, MIDI_CTL_MSB_MAIN_VOLUME, d->m_volume[channel]);
            emit volumeChanged( channel, value );
        } else if ( channel == -1 ) {
            for (int chan = 0; chan < MIDI_CHANNELS; ++chan) {
                d->m_volumeShift[chan] = value;
                sendController(chan, MIDI_CTL_MSB_MAIN_VOLUME, d->m_volume[chan]);
                emit volumeChanged( chan, value );
            }
        }
    }

    static QString getEndpointName(MIDIEndpointRef endpoint)
    {
        QString result;
        CFStringRef str = 0;
        MIDIObjectGetStringProperty (endpoint, kMIDIPropertyName, &str);
        if (str != 0) {
            result = CFStringToQString(str);
            CFRelease(str);
            str = 0;
        }
        MIDIEntityRef entity = 0;
        MIDIEndpointGetEntity(endpoint, &entity);
        if (entity == 0)
            return result;
        if (result.isEmpty()) {
            MIDIObjectGetStringProperty (entity, kMIDIPropertyName, &str);
            if (str != 0) {
                result = CFStringToQString(str);
                CFRelease(str);
                str = 0;
            }
        }
        MIDIDeviceRef device = 0;
        MIDIEntityGetDevice (entity, &device);
        if (device == 0)
            return result;
        MIDIObjectGetStringProperty (device, kMIDIPropertyName, &str);
        if (str != 0) {
            QString s = CFStringToQString(str);
            CFRelease (str);
            str = 0;
            if (!result.startsWith(s, Qt::CaseInsensitive) )
                result = (s + ' ' + result).trimmed();
        }
        return result;
    }

    void MacMIDIOutput::reloadDeviceList()
    {
        int num = MIDIGetNumberOfDestinations();
        d->m_outputDevices.clear();
        for (int i = 0; i < num; ++i) {
            QString result;
            MIDIEndpointRef dest = MIDIGetDestination( i );
            if (dest != 0) {
                QString name = getEndpointName(dest);
                if ( d->m_clientFilter &&
                     name.contains(QLatin1String("IAC"), Qt::CaseSensitive) )
                    continue;
                d->m_outputDevices << name;
            }
        }
        if (!d->m_currentOutput.isEmpty() &&
            !d->m_outputDevices.contains(d->m_currentOutput)) {
            d->m_currentOutput.clear();
            d->m_destination = NULL;
            emit outputDeviceChanged(d->m_currentOutput);
        }
    }

    bool MacMIDIOutput::setOutputDevice(int index)
    {
        if (index >= 0 && index < d->m_outputDevices.count())
            return setOutputDeviceName(d->m_outputDevices[index]);
        return false;
    }

    bool MacMIDIOutput::setOutputDeviceName(const QString &newOutputDevice)
    {
        int index;
        QStringList allOutputDevices;
        int num = MIDIGetNumberOfDestinations();
        for (int i = 0; i < num; ++i) {
            MIDIEndpointRef dest = MIDIGetDestination( i );
            if (dest != 0)
               allOutputDevices << getEndpointName( dest );
        }
        index = allOutputDevices.indexOf(newOutputDevice);
        if (index < 0)
            return false;
        d->m_destination = MIDIGetDestination( index );
        d->m_currentOutput = newOutputDevice;
        return true;
    }

    MIDIClientRef MacMIDIOutput::client() const
    {
        return d->m_client;
    }

    void MacMIDIOutput::setMuted(int channel, bool mute)
    {
        if (channel >= 0 && channel < MIDI_CHANNELS) {
            if (d->m_muted[channel] != mute) {
                if (mute) {
                    sendController(channel, MIDI_CTL_ALL_NOTES_OFF, 0);
                    sendController(channel, MIDI_CTL_ALL_SOUNDS_OFF, 0);
                }
                d->m_muted[channel] = mute;
                emit mutedChanged( channel, mute );
            }
        }
    }

    void MacMIDIOutput::setLocked(int channel, bool lock)
    {
        if (channel >= 0 && channel < MIDI_CHANNELS) {
            if (d->m_locked[channel] != lock) {
                d->m_locked[channel] = lock;
                if (lock)
                    d->m_lockedpgm[channel] = d->m_lastpgm[channel];
                emit lockedChanged( channel, lock );
            }
        }
    }

    void MacMIDIOutput::setPitchShift(int amt)
    {
        if (d->m_pitchShift != amt) {
            allNotesOff();
            d->m_pitchShift = amt;
        }
    }

    void MacMIDIOutput::setResetMessage(const QByteArray& msg)
    {
        d->m_resetMessage = msg;
    }

    /* Realtime MIDI slots */

    void MacMIDIOutput::allNotesOff()
    {
        quint8 data[3];
        quint8 buf[2048];
        MIDIPacketList* pktlist = (MIDIPacketList*) &buf;
        MIDIPacket* packet = MIDIPacketListInit(pktlist);
        for(int chan = 0; chan < MIDI_CHANNELS && packet != NULL; ++chan) {
            data[0] = MIDI_STATUS_CONTROLCHANGE | (chan & 0x0f);
            data[1] = MIDI_CTL_ALL_NOTES_OFF;
            data[2] = 0;
            packet = MIDIPacketListAdd(pktlist, sizeof(buf), packet, 0,
                      sizeof(data), data);
            if (packet != NULL) {
                data[1] = MIDI_CTL_ALL_SOUNDS_OFF;
                packet = MIDIPacketListAdd(pktlist, sizeof(buf), packet, 0,
                           sizeof(data), data);
            }
        }
        if (packet != NULL)
            sendEvents(pktlist);
    }

    void MacMIDIOutput::resetControllers()
    {
        quint8 data[3];
        quint8 buf[2048];
        MIDIPacketList* pktlist = (MIDIPacketList*) &buf;
        MIDIPacket* packet = MIDIPacketListInit(pktlist);
        for(int chan = 0; chan < MIDI_CHANNELS && packet != NULL; ++chan) {
            data[0] = MIDI_STATUS_CONTROLCHANGE | (chan & 0x0f);
            data[1] = MIDI_CTL_RESET_CONTROLLERS;
            data[2] = 0;
            packet = MIDIPacketListAdd(pktlist, sizeof(buf), packet, 0,
                      sizeof(data), data);
            if (packet != NULL) {
                data[1] = MIDI_CTL_MSB_MAIN_VOLUME;
                data[2] = 100;
                packet = MIDIPacketListAdd(pktlist, sizeof(buf), packet, 0,
                           sizeof(data), data);
            }
        }
        if (packet != NULL)
            sendEvents(pktlist);
    }

    void MacMIDIOutput::sendResetMessage()
    {
        if (d->m_resetMessage.size() > 0)
            sendSysexEvent(d->m_resetMessage);
    }

    void MacMIDIOutput::sendNoteOn(int chan, int note, int vel)
    {
        if ( d->m_muted[chan] )
            return;
        quint8 data[3];
        MIDIPacketList pktlist ;
        MIDIPacket* packet = MIDIPacketListInit(&pktlist);
        data[0] = MIDI_STATUS_NOTEON | (chan & 0x0f);
        data[1] = note;
        data[2] = vel;
        packet = MIDIPacketListAdd(&pktlist, sizeof(pktlist), packet, 0,
            sizeof(data), data);
        if (packet != NULL)
            sendEvents(&pktlist);
    }

    void MacMIDIOutput::sendNoteOff(int chan, int note, int vel)
    {
        if ( d->m_muted[chan] )
            return;
        quint8 data[3];
        MIDIPacketList pktlist ;
        MIDIPacket* packet = MIDIPacketListInit(&pktlist);
        data[0] = MIDI_STATUS_NOTEOFF | (chan & 0x0f);
        data[1] = note;
        data[2] = vel;
        packet = MIDIPacketListAdd(&pktlist, sizeof(pktlist), packet, 0,
            sizeof(data), data);
        if (packet != NULL)
            sendEvents(&pktlist);
    }

    void MacMIDIOutput::sendController(int chan, int control, int value)
    {
        if ( d->m_muted[chan] )
            return;
        quint8 data[3];
        MIDIPacketList pktlist ;
        MIDIPacket* packet = MIDIPacketListInit(&pktlist);
        data[0] = MIDI_STATUS_CONTROLCHANGE | (chan & 0x0f);
        data[1] = control;
        data[2] = value;
        packet = MIDIPacketListAdd(&pktlist, sizeof(pktlist), packet, 0,
            sizeof(data), data);
        if (packet != NULL)
            sendEvents(&pktlist);
    }

    void MacMIDIOutput::sendKeyPressure(int chan, int note, int value)
    {
        if ( d->m_muted[chan] )
            return;
        quint8 data[3];
        MIDIPacketList pktlist ;
        MIDIPacket* packet = MIDIPacketListInit(&pktlist);
        data[0] = MIDI_STATUS_KEYPRESURE | (chan & 0x0f);
        data[1] = note;
        data[2] = value;
        packet = MIDIPacketListAdd(&pktlist, sizeof(pktlist), packet, 0,
            sizeof(data), data);
        if (packet != NULL)
            sendEvents(&pktlist);
    }

    void MacMIDIOutput::sendProgram(int chan, int program)
    {
        if ( d->m_muted[chan] )
            return;
        if ( d->m_locked[chan] )
            return;
        quint8 data[2];
        MIDIPacketList pktlist ;
        MIDIPacket* packet = MIDIPacketListInit(&pktlist);
        data[0] = MIDI_STATUS_PROGRAMCHANGE | (chan & 0x0f);
        data[1] = program;
        packet = MIDIPacketListAdd(&pktlist, sizeof(pktlist), packet, 0,
            sizeof(data), data);
        if (packet != NULL)
            sendEvents(&pktlist);
    }

    void MacMIDIOutput::sendChannelPressure(int chan, int value)
    {
        if ( d->m_muted[chan] )
            return;
        quint8 data[2];
        MIDIPacketList pktlist ;
        MIDIPacket* packet = MIDIPacketListInit(&pktlist);
        data[0] = MIDI_STATUS_CHANNELPRESSURE | (chan & 0x0f);
        data[1] = value;
        packet = MIDIPacketListAdd(&pktlist, sizeof(pktlist), packet, 0,
            sizeof(data), data);
        if (packet != NULL)
            sendEvents(&pktlist);
    }

    void MacMIDIOutput::sendPitchBend(int chan, int value)
    {
        if ( d->m_muted[chan] )
            return;
        quint16 val = value + 8192; // value between -8192 and +8191
        quint8 data[3];
        MIDIPacketList pktlist ;
        MIDIPacket* packet = MIDIPacketListInit(&pktlist);
        data[0] = MIDI_STATUS_PITCHBEND | (chan & 0x0f);
        data[1] = val % 0x80; // LSB
        data[2] = val / 0x80; // MSB
        packet = MIDIPacketListAdd(&pktlist, sizeof(pktlist), packet, 0,
            sizeof(data), data);
        if (packet != NULL)
            sendEvents(&pktlist);
    }

    void MacMIDIOutput::sendSysexEvent(const QByteArray& data)
    {
        quint8 buf[4096];
        if (data.size() > 4096)
            return;
        MIDIPacketList* pktlist = (MIDIPacketList*) &buf;
        MIDIPacket* packet = MIDIPacketListInit(pktlist);
        packet = MIDIPacketListAdd(pktlist, sizeof(buf), packet, 0,
            data.size(), (const Byte*)data.data());
        if (packet != NULL)
            sendEvents(pktlist);
    }

    void MacMIDIOutput::sendEvents( const MIDIPacketList* events,
                                    bool discardable )
    {
        quint8 buf[4096];
        MIDIPacketList* pktlist = (MIDIPacketList*) &buf;
        MIDIPacket *curpacket = MIDIPacketListInit(pktlist);
        const MIDIPacket *srcpacket = events->packet;
        for ( int i = 0; i < events->numPackets; ++i ) {
            MIDIPacket dstpacket = *srcpacket;
            d->transform(&dstpacket);
            quint8 status = dstpacket.data[0] & 0xf0;
            quint8 chan = dstpacket.data[0] & 0x0f;
            bool reject = ( status < MIDI_STATUS_NOTEOFF ) ||
                          ( status >= MIDI_STATUS_SYSEX ) ||
                          ((status < MIDI_STATUS_SYSEX) &&
                            discardable && d->m_muted[chan]);
            if ( discardable && status == MIDI_STATUS_PROGRAMCHANGE )
                 reject |= d->m_locked[chan];
            if (!reject)
               curpacket = MIDIPacketListAdd(pktlist, sizeof(buf), curpacket, 0,
                           dstpacket.length, (const Byte*)&dstpacket.data);
            srcpacket = MIDIPacketNext(srcpacket);
        }
        MIDISend(d->m_outPort, d->m_destination, pktlist);
    }

    void MacMIDIOutput::sendInitialProgram(int chan, int program)
    {
        int pgm(d->m_locked[chan] ? d->m_lockedpgm[chan] : program);
        if (pgm > -1) {
            quint8 data[2];
            MIDIPacketList pktlist ;
            MIDIPacket* packet = MIDIPacketListInit(&pktlist);
            data[0] = MIDI_STATUS_PROGRAMCHANGE | (chan & 0x0f);
            data[1] = pgm;
            packet = MIDIPacketListAdd(&pktlist, sizeof(pktlist), packet, 0,
                sizeof(data), data);
            if (packet != NULL)
                sendEvents(&pktlist, false);
        }
    }

}
