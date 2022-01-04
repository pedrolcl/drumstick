/*
    Drumstick RT Backend
    Copyright (C) 2009-2022 Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include "macmidioutput.h"
#include "maccommon.h"

#include <QObject>
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

        MIDIConnection m_currentOutput;
        QString m_publicName;
        QStringList m_excludedNames;
        QList<MIDIConnection> m_outputDevices;
        bool m_status;
        QStringList m_diagnostics;

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

        void registerStatus(const QString& context, const OSStatus status)
        {
            if (status != noErr) {
                m_diagnostics << QString("%1 error: %2").arg(context, status);
                m_diagnostics << getErrorTextFromOSStatus(status);
            }
        }

        void internalCreate(CFStringRef name)
        {
            OSStatus result = noErr;
            m_status = false;
            m_diagnostics.clear();
            result = MIDIClientCreate( name, NULL, NULL, &m_client );
            if ( result != noErr ) {
                registerStatus("MIDIClientCreate()", result);
                return;
            }
            result = MIDISourceCreate( m_client, name, &m_endpoint );
            if ( result != noErr ) {
                registerStatus("MIDISourceCreate()", result);
                return;
            }
            result = MIDIOutputPortCreate( m_client, name, &m_port );
            if (result != noErr) {
                registerStatus("MIDIOutputPortCreate()", result);
                return;
            }            
            reloadDeviceList(true);
            m_status = (result == noErr);
        }

        virtual ~MacMIDIOutputPrivate()
        {
            internalDispose();
        }

        void internalDispose()
        {
            OSStatus result = noErr;
            m_status = false;
            m_diagnostics.clear();
            if (m_port != 0) {
                result = MIDIPortDispose( m_port );
                if (result != noErr) {
                    registerStatus("MIDIPortDispose()", result);
                    m_port = 0;
                }
            }
            if (m_endpoint != 0) {
                result = MIDIEndpointDispose( m_endpoint );
                if (result != noErr) {
                    registerStatus("MIDIEndpointDispose()", result);
                    m_endpoint = 0;
                }
            }
            if (m_client != 0) {
                result = MIDIClientDispose( m_client );
                if (result != noErr) {
                    registerStatus("MIDIClientDispose()", result);
                    m_client = 0;
                }
            }
            m_status = (result == noErr);
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
                         name.contains(QStringLiteral("IAC"), Qt::CaseSensitive) )
                        continue;
                    if ( name.contains (m_publicName) )
                        continue;
                    foreach ( const QString& n, m_excludedNames ) {
                        if ( name.contains(n) ) {
                            excluded = true;
                            break;
                        }
                    }
                    if (!excluded) {
                        m_outputDevices << MIDIConnection(name, i);
                    }
                }
            }
            if (!m_currentOutput.first.isEmpty() && m_destination != 0 &&
                !m_outputDevices.contains(m_currentOutput)) {
                m_currentOutput = MIDIConnection();
                m_destination = 0;
            }
        }

        void sendEvents( const MIDIPacketList* events )
        {
            MIDIReceived(m_endpoint, events);
            if (m_destination != 0)
                MIDISend(m_port, m_destination, events);
        }

        bool open(const MIDIConnection& conn)
        {
            m_destination = MIDIGetDestination( conn.second.toInt() );
            m_currentOutput = conn;
            return true;
        }

        void close()
        {
            m_destination = 0;
            m_currentOutput = MIDIConnection();
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
        return QLatin1String("CoreMIDI");
    }

    QString MacMIDIOutput::publicName()
    {
        return d->m_publicName;
    }

    void MacMIDIOutput::setPublicName(QString name)
    {
        d->setPublicName(name);
    }

    QList<MIDIConnection> MacMIDIOutput::connections(bool advanced)
    {
        d->reloadDeviceList(advanced);
        return d->m_outputDevices;
    }

    void MacMIDIOutput::setExcludedConnections(QStringList conns)
    {
        d->m_excludedNames = conns;
    }

    void MacMIDIOutput::open(const MIDIConnection& name)
    {
        d->open(name);
    }

    void MacMIDIOutput::close()
    {
        d->close();
    }

    MIDIConnection MacMIDIOutput::currentConnection()
    {
        return d->m_currentOutput;
    }

    /* Realtime MIDI slots */

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

    QStringList MacMIDIOutput::getDiagnostics()
    {
        return d->m_diagnostics;
    }

    bool MacMIDIOutput::getStatus()
    {
        return d->m_status;
    }

}}
