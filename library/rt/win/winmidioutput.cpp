/*
    Drumstick Karaoke Windows Backend
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

#include <QDebug>
#include <QStringList>
#include <QByteArray>
#include <QVarLengthArray>
#include <qmath.h>
#include "winmidioutput.h"

namespace drumstick {
namespace rt {

    union WinMIDIPacket {
        WinMIDIPacket() : dwPacket(0) {}
        DWORD dwPacket;
        quint8 data[sizeof(DWORD)];
    };

    void CALLBACK midiCallback( HMIDISTRM hms, UINT uMsg, DWORD dwUser,
                                DWORD dwParam1, DWORD dwParam2 )
    {
        WinMIDIOutput* output = (WinMIDIOutput*) dwUser;
        switch( uMsg ) {
        case MOM_DONE:
            output->doneHeader( (LPMIDIHDR) dwParam1 );
            break;
        case MOM_POSITIONCB:
            output->streamPosition( (LPMIDIHDR) dwParam1 );
            break;
        case MOM_OPEN:
        case MOM_CLOSE:
            break;
        default:
            qDebug() << "unknown:" << hex << uMsg;
            break;
        }
    }

    class WinMIDIOutput::WinMIDIOutputPrivate {
    public:
        WinMIDIOutputPrivate():
            m_outHandle(0),
            m_obj(0),
            m_pitchShift(0),
            m_clientFilter(true),
            m_tickInterval(0),
            m_timeSkew(1.0),
            m_currentTempo(120),
            m_currentOutput(-1),
            m_pendingReset(false),
            m_pendingTempo(false),
            m_songIterator(0)
        {
            clear();
        }

        virtual ~WinMIDIOutputPrivate()
        {
            if (m_songIterator != NULL)
                delete m_songIterator;
        }

        void clear()
        {
            for (int chan = 0; chan < MIDI_CHANNELS; ++chan) {
                m_lastpgm[chan] = 0;
                m_lockedpgm[chan] = 0;
                m_volumeShift[chan] = 1.0;
                m_volume[chan] = 100;
                m_muted[chan] = false;
                m_locked[chan] = false;
            }
            m_pitchShift = 0;
            m_tickInterval = 0;
            m_timeSkew = 1.0;
            m_currentTempo = 120;
            m_pendingReset = false;
            m_pendingTempo = false;
        }

        void transformControllerEvent(WinMIDIPacket &packet)
        {
            int param = packet.data[1];
            if (param == MIDI_CTL_MSB_MAIN_VOLUME) {
                int chan = packet.data[0] & MIDI_CHANNEL_MASK;
                int value = packet.data[2];
                m_volume[chan] = value;
                value = floor(value * m_volumeShift[chan]);
                if (value < 0) value = 0;
                if (value > 127) value = 127;
                packet.data[2] = value;
            }
        }

        void transformNoteEvent(WinMIDIPacket &packet)
        {
            int note, channel;
            channel = packet.data[0] & MIDI_CHANNEL_MASK;
            note = packet.data[1];
            if (channel != MIDI_GM_DRUM_CHANNEL) {
                note += m_pitchShift;
                while (note > 127) note -= 12;
                while (note < 0) note += 12;
                packet.data[1] = note;
            }
        }

        void transformProgramEvent(WinMIDIPacket &packet)
        {
            int channel = packet.data[0] & MIDI_CHANNEL_MASK;
            m_lastpgm[channel] = packet.data[1];
        }

        void transform(WinMIDIPacket &packet)
        {
            quint8 status = packet.data[0] & MIDI_STATUS_MASK;
            //quint8 chan = packet.data[0] & MIDI_CHANNEL_MASK;
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
                    packet.data[0] = status + channel;
            }*/
        }

        HMIDISTRM m_outHandle;
        WinMIDIObject *m_obj;
        int m_pitchShift;
        bool m_clientFilter;
        qint32 m_tickInterval;
        qreal m_timeSkew;
        int m_currentTempo;
        int m_currentOutput;
        QMap<int,QString> m_outputDevices;
        int m_lastpgm[MIDI_CHANNELS];
        int m_lockedpgm[MIDI_CHANNELS];
        qreal m_volumeShift[MIDI_CHANNELS];
        int m_volume[MIDI_CHANNELS];
        bool m_muted[MIDI_CHANNELS];
        bool m_locked[MIDI_CHANNELS];
        bool m_pendingReset;
        bool m_pendingTempo;
        QByteArray m_resetMessage;
        SongIterator *m_songIterator;

        MIDIHDR m_midiSysexHdr;
        QByteArray m_sysexBuffer;
        MIDIHDR m_midiStreamHdr;
        QVarLengthArray<DWORD,STREAM_BUFMAX> m_streamBuffer;
    };

    WinMIDIOutput::WinMIDIOutput(QObject *parent) :
        MIDIOutput(parent), d(new WinMIDIOutputPrivate)
    {
        reloadDeviceList();
    }

    WinMIDIOutput::~WinMIDIOutput()
    {
        closeStream();
        delete d;
    }

    void WinMIDIOutput::initialize(WinMIDIObject *object)
    {
        d->m_obj = object;
    }

    /* Stream IO Methods */

    QString mmErrorString(MMRESULT err)
    {
        QString errstr;
#ifdef UNICODE
        WCHAR buffer[1024];
        midiOutGetErrorText(err, &buffer[0], sizeof(buffer));
        errstr = QString::fromUtf16((const ushort*)buffer);
#else
        char buffer[1024];
        midiOutGetErrorText(err, &buffer[0], sizeof(buffer));
        errstr = QString::fromLocal8Bit(buffer);
#endif
        return errstr;
    }

    void WinMIDIOutput::closeStream()
    {
        MMRESULT res;
        if (d->m_outHandle != 0) {
            res = midiOutReset( (HMIDIOUT) d->m_outHandle );
            if (res != MMSYSERR_NOERROR)
                qDebug() << "midiOutReset() err:" << mmErrorString(res);
            res = midiStreamClose( d->m_outHandle );
            if (res != MMSYSERR_NOERROR)
                qDebug() << "midiStreamClose() err:" << mmErrorString(res);
            d->m_outHandle = 0;
        }
    }

    bool WinMIDIOutput::openStream(unsigned int dev)
    {
        MMRESULT res;
        unsigned int max = midiOutGetNumDevs();
        if ((dev >= max) || (dev < 0))
            return false;
        if (d->m_outHandle != 0)
            closeStream();
        res = midiStreamOpen( &d->m_outHandle, &dev, 1,
                (DWORD) midiCallback, (DWORD) this, CALLBACK_FUNCTION);
        if (res != MMSYSERR_NOERROR) {
            qDebug() << "midiStreamOpen() err:" << mmErrorString(res);
            return false;
        }
        return true;
    }

    void inline WinMIDIOutput::outEvent(int &lastTick, bool &first, SongEvent *ev)
    {
        int i, j;
        quint8 status;
        quint8 chan;
        bool reject(false);
        DWORD delta = 0;
        WinMIDIPacket packet, lpack;
        packet.dwPacket = 0;
        delta = ev->getTick() - lastTick;
        switch (ev->getType()) {
        case SHORT_EVENT:
            packet.dwPacket = ev->getPacket();
            d->transform(packet);
            status = packet.data[0] & MIDI_STATUS_MASK;
            chan = packet.data[0] & MIDI_CHANNEL_MASK;
            reject = ( ( status < MIDI_STATUS_NOTEOFF ) ||
                       ( status >= MIDI_STATUS_SYSEX ) ||
                       ((status < MIDI_STATUS_SYSEX) &&
                         d->m_muted[chan]) ||
                       ((status == MIDI_STATUS_PROGRAMCHANGE) &&
                         d->m_locked[chan]) );
            if (reject)
                packet.dwPacket = 0;
            else
                packet.data[3] = MEVT_SHORTMSG;
            break;
        case TEMPO_EVENT:
            d->m_currentTempo = ev->getPacket();
            packet.dwPacket = (DWORD) (d->m_currentTempo / d->m_timeSkew);
            packet.data[3] = MEVT_TEMPO;
            break;
        case TICK_EVENT:
            packet.dwPacket = 0;
            packet.data[3] = MEVT_NOP;
            break;
        case LYRIC_EVENT:
            packet.dwPacket = 1;
            packet.data[3] = MEVT_NOP;
            break;
        case BEAT_EVENT:
            packet.dwPacket = ev->getPacket();
            packet.data[3] = MEVT_NOP;
            break;
        case SYSEX_EVENT:
            lpack.dwPacket = ev->getDataLength();
            lpack.data[3] = MEVT_LONGMSG;
            if (first) {
                lpack.dwPacket |= MEVT_F_CALLBACK;
                first = false;
            }
            d->m_streamBuffer.append(delta);
            d->m_streamBuffer.append(0);
            d->m_streamBuffer.append(lpack.dwPacket);
            for(i=0; i < ev->getDataLength(); i+=4) {
                lpack.dwPacket = 0;
                for(j=0; j<4; ++j)
                    if (ev->getDataLength() > i+j)
                        lpack.data[j] = ev->getData()[i+j];
                d->m_streamBuffer.append(lpack.dwPacket);
            }
            lastTick = ev->getTick();
            break;
        default:
            qDebug() << "ev. ignored:" << ev->getType();
            break;
        }
        if (packet.dwPacket != 0) {
            if (first || (delta != 0)) {
                packet.dwPacket |= MEVT_F_CALLBACK;
                first = false;
            }
            d->m_streamBuffer.append(delta);
            d->m_streamBuffer.append(0);
            d->m_streamBuffer.append(packet.dwPacket);
            lastTick = ev->getTick();
        }
    }

    void WinMIDIOutput::songOutput()
    {
        MMRESULT res;
        SongEvent* ev;
        bool first = true;
        int initialTick = 0, lastTick = 0, limit = 0;
        d->m_streamBuffer.clear();
        // Fill the stream buffer
        if (d->m_songIterator->hasPrevious()) {
            ev = d->m_songIterator->peekPrevious();
            initialTick = ev->getTick();
        }
        lastTick = initialTick;
        limit = initialTick + d->m_tickInterval;
        if (d->m_pendingTempo) {
            ev = new SongEvent(TEMPO_EVENT, d->m_currentTempo);
            ev->setTick(initialTick);
            outEvent(lastTick, first, ev);
            delete ev;
            d->m_pendingTempo = false;
        }
        if (d->m_pendingReset) {
            ev = new SongEvent(SYSEX_EVENT, d->m_resetMessage);
            ev->setTick(initialTick);
            outEvent(lastTick, first, ev);
            delete ev;
            d->m_pendingReset = false;
        }
        while (d->m_songIterator->hasNext() && d->m_streamBuffer.size() < STREAM_BUFMAX) {
            ev = d->m_songIterator->peekNext();
            if (ev->getTick() > limit && !d->m_streamBuffer.isEmpty())
                break;
            ev = d->m_songIterator->next();
            outEvent(lastTick, first, ev);
        }
        if (!d->m_streamBuffer.isEmpty()) {
            d->m_midiStreamHdr.dwBufferLength = d->m_streamBuffer.size()*sizeof(DWORD);
            d->m_midiStreamHdr.dwBytesRecorded = d->m_streamBuffer.size()*sizeof(DWORD);
            d->m_midiStreamHdr.lpData = (LPSTR) d->m_streamBuffer.data();
            d->m_midiStreamHdr.dwUser = initialTick;
            d->m_midiStreamHdr.dwFlags = MHDR_ISSTRM;
            res = midiOutPrepareHeader( (HMIDIOUT) d->m_outHandle,
                                        &d->m_midiStreamHdr,
                                        sizeof(MIDIHDR) );
            if (res != MMSYSERR_NOERROR)
                qDebug() << "midiStreamOut() err:" << mmErrorString(res);
            res = midiStreamOut( d->m_outHandle,
                                 &d->m_midiStreamHdr,
                                 sizeof(MIDIHDR) );
            if (res != MMSYSERR_NOERROR)
                qDebug() << "midiStreamOut() err:" << mmErrorString(res);
        }
    }

    void WinMIDIOutput::doneHeader( LPMIDIHDR lpMidiHdr )
    {
        MMRESULT res;
        res = midiOutUnprepareHeader( (HMIDIOUT) d->m_outHandle,
                                      lpMidiHdr, sizeof(MIDIHDR) );
        if (res != MMSYSERR_NOERROR)
            qDebug() << "midiOutUnprepareHeader() err:" << mmErrorString(res);
        if ((lpMidiHdr->dwFlags & MHDR_ISSTRM) == 0)
            return; // sysex header?
        if (d->m_songIterator->hasNext())
            songOutput();
        else
            emit finished();
    }

    void WinMIDIOutput::streamPosition( LPMIDIHDR lpMidiHdr )
    {
        WinMIDIPacket packet;
        MIDIEVENT* event = 0;
        qint64 prev = 0, ticks = lpMidiHdr->dwUser;
        unsigned int offset = lpMidiHdr->dwOffset;
        while (offset < lpMidiHdr->dwBytesRecorded) {
            event = (MIDIEVENT*)(lpMidiHdr->lpData + offset);
            ticks += event->dwDeltaTime;
            if (prev != 0 && prev != ticks)
                return;
            prev = ticks;
            packet.dwPacket = event->dwEvent;
            if ((event->dwEvent & MEVT_F_LONG) != 0) {
                int paramsize = MEVT_EVENTPARM(event->dwEvent);
                int remainder = paramsize % sizeof(DWORD);
                int padding = 0;
                if (paramsize != 0 && remainder != 0)
                    padding = sizeof(DWORD) - remainder;
                offset += (3*sizeof(DWORD) + paramsize + padding);
            } else {
                offset += 3*sizeof(DWORD);
                d->m_obj->emitShortEvSignals(ticks, packet);
            }
        }
    }

    void WinMIDIOutput::setTimeDivision(int division)
    {
        MMRESULT res;
        MIDIPROPTIMEDIV mptd;
        mptd.cbStruct = sizeof(MIDIPROPTIMEDIV);
        mptd.dwTimeDiv = division;
        res = midiStreamProperty( d->m_outHandle, (LPBYTE) &mptd,
                                  MIDIPROP_SET | MIDIPROP_TIMEDIV );
        if (res != MMSYSERR_NOERROR)
            qDebug() << "midiStreamProperty() err:" << mmErrorString(res);
    }

    /* Stream IO Methods */

    qreal WinMIDIOutput::volume(int channel) const
    {
        if (channel >=0 && channel < MIDI_CHANNELS)
            return d->m_volumeShift[channel];
        return -1.0;
    }

    int WinMIDIOutput::outputDevice() const
    {
        return d->m_currentOutput;
    }

    QString WinMIDIOutput::outputDeviceName() const
    {
        if (d->m_outputDevices.contains(d->m_currentOutput))
            return d->m_outputDevices[d->m_currentOutput];
        return QString();
    }

    bool WinMIDIOutput::isMuted(int channel) const
    {
        if (channel >= 0 && channel < MIDI_CHANNELS)
            return d->m_muted[channel];
        return false;
    }

    int WinMIDIOutput::pitchShift()
    {
        return d->m_pitchShift;
    }

    QStringList WinMIDIOutput::outputDeviceList(bool basicOnly)
    {
        d->m_clientFilter = basicOnly;
        reloadDeviceList();
        return d->m_outputDevices.values();
    }

    /* SLOTS */

    void WinMIDIOutput::setVolume(int channel, qreal value)
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

    void WinMIDIOutput::reloadDeviceList()
    {
        MMRESULT res;
        MIDIOUTCAPS deviceCaps;
        QString devName;
        unsigned int dev, max = midiOutGetNumDevs();
        d->m_outputDevices.clear();

        for ( dev = 0; dev < max; ++dev) {
            res = midiOutGetDevCaps( dev, &deviceCaps, sizeof(MIDIOUTCAPS));
            if (res != MMSYSERR_NOERROR)
                break;
            if (d->m_clientFilter && (deviceCaps.wTechnology == MOD_MAPPER))
                continue;
#if defined(UNICODE)
            devName = QString::fromWCharArray(deviceCaps.szPname);
#else
            devName = QString::fromLocal8Bit(deviceCaps.szPname);
#endif
            d->m_outputDevices[dev] = devName;
        }
        if (!d->m_clientFilter) {
            dev = MIDI_MAPPER;
            res = midiOutGetDevCaps( dev, &deviceCaps, sizeof(MIDIOUTCAPS));
            if (res == MMSYSERR_NOERROR) {
#if defined(UNICODE)
                devName = QString::fromWCharArray(deviceCaps.szPname);
#else
                devName = QString::fromLocal8Bit(deviceCaps.szPname);
#endif
                d->m_outputDevices[dev] = devName;
            }
        }
    }

    bool WinMIDIOutput::setOutputDevice(int index)
    {
        if (!d->m_outputDevices.contains(index))
            return false;
        if (openStream(index)) {
            d->m_currentOutput = index;
            emit outputDeviceChanged(d->m_outputDevices[index]);
            return true;
        }
        return false;
    }

    bool WinMIDIOutput::setOutputDeviceName( const QString& newOutputDevice )
    {
        int index = -1;
        QMap<int,QString>::ConstIterator it;
        for( it = d->m_outputDevices.constBegin();
             it != d->m_outputDevices.constEnd(); ++it )
            if (it.value() == newOutputDevice) {
                index = it.key();
                break;
            }
        return setOutputDevice(index);
    }

    void WinMIDIOutput::setMuted(int channel, bool mute)
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

    void WinMIDIOutput::setLocked(int channel, bool lock)
    {
        if (channel >= 0 && channel < MIDI_CHANNELS) {
            if (d->m_locked[channel] != lock) {
                d->m_locked[channel] = lock;
                if (lock) {
                    d->m_lockedpgm[channel] = d->m_lastpgm[channel];
                }
                emit lockedChanged( channel, lock );
            }
        }
    }

    void WinMIDIOutput::setPitchShift(int amt)
    {
        if (d->m_pitchShift != amt) {
            allNotesOff();
            d->m_pitchShift = amt;
            //qDebug() << "pitchShift:" << d->m_pitchShift;
        }
    }

    void WinMIDIOutput::setResetMessage(const QByteArray& msg)
    {
        d->m_resetMessage = msg;
    }

    /* Realtime MIDI slots */

    void WinMIDIOutput::allNotesOff()
    {
        for(int chan = 0; chan < MIDI_CHANNELS; ++chan) {
            sendController(chan, MIDI_CTL_ALL_NOTES_OFF, 0);
            sendController(chan, MIDI_CTL_ALL_SOUNDS_OFF, 0);
        }
    }

    void WinMIDIOutput::resetControllers()
    {
        for(int chan = 0; chan < MIDI_CHANNELS; ++chan) {
            sendController(chan, MIDI_CTL_RESET_CONTROLLERS, 0);
            sendController(chan, MIDI_CTL_MSB_MAIN_VOLUME, 100);
        }
    }

    void WinMIDIOutput::sendResetMessage()
    {
        if (d->m_resetMessage.size() > 0) {
            sendSysexEvent(d->m_resetMessage);
            d->m_pendingReset = true;
        }
    }

    void WinMIDIOutput::sendShortMessage(WinMIDIPacket &packet, bool discardable)
    {
        MMRESULT res;
        quint8 status;
        quint8 chan;
        bool reject(false);
        d->transform(packet);
        status = packet.data[0] & MIDI_STATUS_MASK;
        chan = packet.data[0] & MIDI_CHANNEL_MASK;
        reject = ( ( status < MIDI_STATUS_NOTEOFF ) ||
                   ( status >= MIDI_STATUS_SYSEX ) ||
                   ((status < MIDI_STATUS_SYSEX) &&
                     d->m_muted[chan] && discardable) );
        if ( discardable && (status == MIDI_STATUS_PROGRAMCHANGE) )
            reject |= d->m_locked[chan];
        if (!reject) {
            res = midiOutShortMsg( (HMIDIOUT)d->m_outHandle, packet.dwPacket );
            if ( res != MMSYSERR_NOERROR )
                qDebug() << "midiOutShortMsg() err:" << mmErrorString(res);
        }
    }

    void WinMIDIOutput::sendNoteOn(int chan, int note, int vel)
    {
        WinMIDIPacket packet;
        packet.data[0] = MIDI_STATUS_NOTEON | (chan & MIDI_CHANNEL_MASK);
        packet.data[1] = note;
        packet.data[2] = vel;
        sendShortMessage(packet);
    }

    void WinMIDIOutput::sendNoteOff(int chan, int note, int vel)
    {
        WinMIDIPacket packet;
        packet.data[0] = MIDI_STATUS_NOTEOFF | (chan & MIDI_CHANNEL_MASK);
        packet.data[1] = note;
        packet.data[2] = vel;
        sendShortMessage(packet);
    }

    void WinMIDIOutput::sendController(int chan, int control, int value)
    {
        WinMIDIPacket packet;
        packet.data[0] = MIDI_STATUS_CONTROLCHANGE | (chan & MIDI_CHANNEL_MASK);
        packet.data[1] = control;
        packet.data[2] = value;
        sendShortMessage(packet);
    }

    void WinMIDIOutput::sendKeyPressure(int chan, int note, int value)
    {
        WinMIDIPacket packet;
        packet.data[0] = MIDI_STATUS_KEYPRESURE | (chan & MIDI_CHANNEL_MASK);
        packet.data[1] = note;
        packet.data[2] = value;
        sendShortMessage(packet);
    }

    void WinMIDIOutput::sendProgram(int chan, int program)
    {
        WinMIDIPacket packet;
        packet.data[0] = MIDI_STATUS_PROGRAMCHANGE | (chan & MIDI_CHANNEL_MASK);
        packet.data[1] = program;
        sendShortMessage(packet);
    }

    void WinMIDIOutput::sendChannelPressure(int chan, int value)
    {
        WinMIDIPacket packet;
        packet.data[0] = MIDI_STATUS_CHANNELPRESSURE | (chan & MIDI_CHANNEL_MASK);
        packet.data[1] = value;
        sendShortMessage(packet);
    }

    void WinMIDIOutput::sendPitchBend(int chan, int value)
    {
        WinMIDIPacket packet;
        packet.data[0] = MIDI_STATUS_PITCHBEND | (chan & MIDI_CHANNEL_MASK);
        packet.data[1] = MIDI_LSB(value);
        packet.data[2] = MIDI_MSB(value);
        sendShortMessage(packet);
    }

    void WinMIDIOutput::sendSysexEvent(const QByteArray& data)
    {
        MMRESULT res;
        d->m_sysexBuffer = data;
        d->m_midiSysexHdr.lpData = (LPSTR) d->m_sysexBuffer.data();
        d->m_midiSysexHdr.dwBufferLength = d->m_sysexBuffer.size();
        d->m_midiSysexHdr.dwBytesRecorded = d->m_sysexBuffer.size();
        d->m_midiSysexHdr.dwFlags = 0;
        d->m_midiSysexHdr.dwUser = 0;
        res = midiOutPrepareHeader( (HMIDIOUT) d->m_outHandle,
                                    &d->m_midiSysexHdr,
                                    sizeof(MIDIHDR) );
        if (res != MMSYSERR_NOERROR)
            qDebug() << "midiOutPrepareHeader() err:" << mmErrorString(res);
        else {
            res = midiOutLongMsg( (HMIDIOUT) d->m_outHandle,
                                  &d->m_midiSysexHdr,
                                  sizeof(MIDIHDR) );
            if (res != MMSYSERR_NOERROR)
                qDebug() << "midiOutLongMsg() err:" << mmErrorString(res);
        }
    }

}}
