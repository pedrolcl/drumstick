/*
    Drumstick RT Backend
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

#include "macmidioutput.h"
#include "maccommon.h"

#include <QObject>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QVarLengthArray>

#include <CoreFoundation/CoreFoundation.h>
#include <CoreMIDI/MIDIServices.h>

namespace drumstick {
namespace rt {

    static CFStringRef DEFAULT_PUBLIC_NAME CFSTR("MIDI Out");

    class MacMIDIOutput::MacMIDIOutputPrivate {
    public:
        MIDIClientRef m_client;
        MIDIPortRef m_port;
        MIDIEndpointRef m_endpoint;
        MIDIEndpointRef m_destination;
        bool m_clientFilter;

        QString m_currentOutput;
        QString m_publicName;
        QStringList m_excludedNames;
        QStringList m_outputDevices;

        MacMIDIOutputPrivate():
            m_client(0),
            m_port(0),
            m_endpoint(0),
            m_destination(0),
            m_clientFilter(true),
            m_publicName(QString::fromCFString(DEFAULT_PUBLIC_NAME))
        {
            internalCreate(DEFAULT_PUBLIC_NAME);
        }

        void internalCreate(CFStringRef name)
        {
            OSStatus result = noErr;
            result = MIDIClientCreate( name, NULL, NULL, &m_client );
            if ( result != noErr ) {
                qDebug() << "MIDIClientCreate() error:" << result;
                return;
            }
            result = MIDISourceCreate( m_client, name, &m_endpoint );
            if ( result != noErr ) {
                qDebug() << "MIDISourceCreate() error:" << result;
                return;
            }
            result = MIDIOutputPortCreate( m_client, name, &m_port );
            if (result != noErr) {
                qDebug() << "MIDIOutputPortCreate() error:" << result;
                return;
            }
            reloadDeviceList(true);
        }

        virtual ~MacMIDIOutputPrivate()
        {
            internalDispose();
        }

        void internalDispose()
        {
            OSStatus result = noErr;
            if (m_port != 0) {
                result = MIDIPortDispose( m_port );
                if (result != noErr) {
                    qDebug() << "MIDIPortDispose() error:" << result;
                    m_port = 0;
                }
            }
            if (m_endpoint != 0) {
                result = MIDIEndpointDispose( m_endpoint );
                if (result != noErr) {
                    qDebug() << "MIDIEndpointDispose() err:" << result;
                    m_endpoint = 0;
                }
            }
            if (m_client != 0) {
                result = MIDIClientDispose( m_client );
                if (result != noErr) {
                    qDebug() << "MIDIClientDispose() error:" << result;
                    m_client = 0;
                }
            }
        }

        void setPublicName(QString name)
        {
            if (m_publicName != name) {
                internalDispose();
                internalCreate(name.toCFString());
                m_publicName = name;
            }
        }

        void reloadDeviceList(bool advanced)
        {
            int num = MIDIGetNumberOfDestinations();
            m_clientFilter = !advanced;
            m_outputDevices.clear();
            for (int i = 0; i < num; ++i) {
                bool excluded = false;
                MIDIEndpointRef dest = MIDIGetDestination( i );
                if (dest != 0) {
                    QString name = getEndpointName(dest);
                    if ( m_clientFilter &&
                         name.contains(QLatin1String("IAC"), Qt::CaseSensitive) )
                        continue;
                    if ( name.contains (m_publicName) )
                        continue;
                    foreach ( const QString& n, m_excludedNames ) {
                        if ( name.contains(n) ) {
                            excluded = true;
                            break;
                        }
                    }
                    if (!excluded)
                        m_outputDevices << name;
                }
            }
            if (!m_currentOutput.isEmpty() && m_destination != 0 &&
                !m_outputDevices.contains(m_currentOutput)) {
                m_currentOutput.clear();
                m_destination = 0;
            }
        }

        void sendEvents( const MIDIPacketList* events )
        {
            MIDIReceived(m_endpoint, events);
            if (m_destination != 0)
                MIDISend(m_port, m_destination, events);
        }

        bool open(const QString &name)
        {
            int index;
            QStringList allOutputDevices;
            int num = MIDIGetNumberOfDestinations();
            for (int i = 0; i < num; ++i) {
                MIDIEndpointRef dest = MIDIGetDestination( i );
                if (dest != 0)
                   allOutputDevices << getEndpointName( dest );
            }
            index = allOutputDevices.indexOf(name);
            if (index < 0)
                return false;
            m_destination = MIDIGetDestination( index );
            m_currentOutput = name;
            return true;
        }

        void close()
        {
            m_destination = 0;
            m_currentOutput.clear();
        }
    };

    MacMIDIOutput::MacMIDIOutput(QObject *parent) :
        MIDIOutput(parent), d(new MacMIDIOutputPrivate)
    {
    }

    MacMIDIOutput::~MacMIDIOutput()
    {
        delete d;
    }

    void MacMIDIOutput::initialize(QSettings *settings)
    {
        Q_UNUSED(settings)
    }

    QString MacMIDIOutput::backendName()
    {
        return QLatin1Literal("CoreMIDI");
    }

    QString MacMIDIOutput::publicName()
    {
        return d->m_publicName;
    }

    void MacMIDIOutput::setPublicName(QString name)
    {
        d->setPublicName(name);
    }

    QStringList MacMIDIOutput::connections(bool advanced)
    {
        d->reloadDeviceList(advanced);
        return d->m_outputDevices;
    }

    void MacMIDIOutput::setExcludedConnections(QStringList conns)
    {
        d->m_excludedNames = conns;
    }

    void MacMIDIOutput::open(QString name)
    {
        d->open(name);
    }

    void MacMIDIOutput::close()
    {
        d->close();
    }

    QString MacMIDIOutput::currentConnection()
    {
        return d->m_currentOutput;
    }

    /* Realtime MIDI slots */

    /*void MacMIDIOutput::allNotesOff()
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
            d->sendEvents(pktlist);
    }*/

    /*void MacMIDIOutput::resetControllers()
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
            d->sendEvents(pktlist);
    }*/

    void MacMIDIOutput::sendNoteOn(int chan, int note, int vel)
    {
        quint8 data[3];
        MIDIPacketList pktlist ;
        MIDIPacket* packet = MIDIPacketListInit(&pktlist);
        data[0] = MIDI_STATUS_NOTEON | (chan & 0x0f);
        data[1] = note;
        data[2] = vel;
        packet = MIDIPacketListAdd(&pktlist, sizeof(pktlist), packet, 0,
            sizeof(data), data);
        if (packet != NULL)
            d->sendEvents(&pktlist);
    }

    void MacMIDIOutput::sendNoteOff(int chan, int note, int vel)
    {
        quint8 data[3];
        MIDIPacketList pktlist ;
        MIDIPacket* packet = MIDIPacketListInit(&pktlist);
        data[0] = MIDI_STATUS_NOTEOFF | (chan & 0x0f);
        data[1] = note;
        data[2] = vel;
        packet = MIDIPacketListAdd(&pktlist, sizeof(pktlist), packet, 0,
            sizeof(data), data);
        if (packet != NULL)
            d->sendEvents(&pktlist);
    }

    void MacMIDIOutput::sendController(int chan, int control, int value)
    {
        quint8 data[3];
        MIDIPacketList pktlist ;
        MIDIPacket* packet = MIDIPacketListInit(&pktlist);
        data[0] = MIDI_STATUS_CONTROLCHANGE | (chan & 0x0f);
        data[1] = control;
        data[2] = value;
        packet = MIDIPacketListAdd(&pktlist, sizeof(pktlist), packet, 0,
            sizeof(data), data);
        if (packet != NULL)
            d->sendEvents(&pktlist);
    }

    void MacMIDIOutput::sendKeyPressure(int chan, int note, int value)
    {
        quint8 data[3];
        MIDIPacketList pktlist ;
        MIDIPacket* packet = MIDIPacketListInit(&pktlist);
        data[0] = MIDI_STATUS_KEYPRESURE | (chan & 0x0f);
        data[1] = note;
        data[2] = value;
        packet = MIDIPacketListAdd(&pktlist, sizeof(pktlist), packet, 0,
            sizeof(data), data);
        if (packet != NULL)
            d->sendEvents(&pktlist);
    }

    void MacMIDIOutput::sendProgram(int chan, int program)
    {
        quint8 data[2];
        MIDIPacketList pktlist ;
        MIDIPacket* packet = MIDIPacketListInit(&pktlist);
        data[0] = MIDI_STATUS_PROGRAMCHANGE | (chan & 0x0f);
        data[1] = program;
        packet = MIDIPacketListAdd(&pktlist, sizeof(pktlist), packet, 0,
            sizeof(data), data);
        if (packet != NULL)
            d->sendEvents(&pktlist);
    }

    void MacMIDIOutput::sendChannelPressure(int chan, int value)
    {
        quint8 data[2];
        MIDIPacketList pktlist ;
        MIDIPacket* packet = MIDIPacketListInit(&pktlist);
        data[0] = MIDI_STATUS_CHANNELPRESSURE | (chan & 0x0f);
        data[1] = value;
        packet = MIDIPacketListAdd(&pktlist, sizeof(pktlist), packet, 0,
            sizeof(data), data);
        if (packet != NULL)
            d->sendEvents(&pktlist);
    }

    void MacMIDIOutput::sendPitchBend(int chan, int value)
    {
        quint16 val = value + 8192; // value between -8192 and +8191
        quint8 data[3];
        MIDIPacketList pktlist ;
        MIDIPacket* packet = MIDIPacketListInit(&pktlist);
        data[0] = MIDI_STATUS_PITCHBEND | (chan & 0x0f);
        data[1] = MIDI_LSB(val); // LSB
        data[2] = MIDI_MSB(val); // MSB
        packet = MIDIPacketListAdd(&pktlist, sizeof(pktlist), packet, 0,
            sizeof(data), data);
        if (packet != NULL)
            d->sendEvents(&pktlist);
    }

    void MacMIDIOutput::sendSysex(const QByteArray& data)
    {
        quint8 buf[4096];
        if (data.size() > 4096)
            return;
        MIDIPacketList* pktlist = (MIDIPacketList*) &buf;
        MIDIPacket* packet = MIDIPacketListInit(pktlist);
        packet = MIDIPacketListAdd(pktlist, sizeof(buf), packet, 0,
            data.size(), (const Byte*)data.data());
        if (packet != NULL)
            d->sendEvents(pktlist);
    }

    void MacMIDIOutput::sendSystemMsg(const int status)
    {
        quint8 data;
        MIDIPacketList pktlist;
        MIDIPacket* packet = MIDIPacketListInit(&pktlist);
        data = status;
        packet = MIDIPacketListAdd(&pktlist, sizeof(pktlist), packet, 0,
            sizeof(data), &data);
        if (packet != NULL)
            d->sendEvents(&pktlist);
    }

}}
