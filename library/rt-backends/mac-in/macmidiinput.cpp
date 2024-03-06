/*
    Drumstick RT Backend
    Copyright (C) 2009-2024 Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include <drumstick/rtmidioutput.h>
#include "macmidiinput.h"
#include "maccommon.h"

#include <QStringList>
#include <QMutex>
#include <QObject>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <QtConcurrent>
#endif
#include <CoreFoundation/CoreFoundation.h>
#include <CoreMIDI/CoreMIDI.h>

namespace drumstick {
namespace rt {

    static CFStringRef DEFAULT_PUBLIC_NAME CFSTR("MIDI In");

    static void MacMIDIReadProc( const MIDIPacketList *pktlist,
                                 void *refCon, void *connRefCon );

    class MacMIDIInputPrivate {
    public:
        MacMIDIInput *m_inp;
        MIDIOutput *m_out;
        MIDIClientRef m_client;
        MIDIPortRef m_port;
        MIDIEndpointRef m_endpoint;
        MIDIEndpointRef m_source;
        bool m_thruEnabled;
        bool m_clientFilter;
        QString m_publicName;
        MIDIConnection m_currentInput;
        QStringList m_excludedNames;
        QList<MIDIConnection> m_inputDevices;
        bool m_status;
        QStringList m_diagnostics;

        explicit MacMIDIInputPrivate(MacMIDIInput *inp) :
            m_inp(inp),
            m_out(nullptr),
            m_client(0),
            m_port(0),
            m_endpoint(0),
            m_source(0),
            m_thruEnabled(false),
            m_clientFilter(true),
            m_publicName(QString::fromCFString(DEFAULT_PUBLIC_NAME))
        {
            internalCreate( DEFAULT_PUBLIC_NAME );
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
            result = MIDIClientCreate( name , nullptr, nullptr, &m_client );
            if (result != noErr) {
                registerStatus("MIDIClientCreate()", result);
                return;
            }
            result = MIDIDestinationCreate ( m_client, name, MacMIDIReadProc, (void*) this, &m_endpoint );
            if (result != noErr) {
                registerStatus("MIDIDestinationCreate()", result);
                return;
            }
            result = MIDIInputPortCreate( m_client, name, MacMIDIReadProc, (void *) this,  &m_port );
            if (result != noErr) {
                registerStatus("MIDIInputPortCreate()", result);
                return;
            }
            reloadDeviceList(true);
        }

        virtual ~MacMIDIInputPrivate()
        {
            internalDispose();
        }

        void internalDispose()
        {
            OSStatus result = noErr;
            if (m_port != 0) {
                result = MIDIPortDispose(m_port);
                if (result != noErr) {
                   registerStatus("MIDIPortDispose()", result);
                    m_port = 0;
                }
            }
            if (m_endpoint != 0) {
                result = MIDIEndpointDispose(m_endpoint);
                if (result != noErr) {
                    registerStatus("MIDIEndpointDispose()", result);
                    m_endpoint = 0;
                }
            }
            if (m_client != 0) {
                result = MIDIClientDispose(m_client);
                if (result != noErr) {
                    registerStatus("MIDIClientDispose()", result);
                    m_client = 0;
                }
            }
        }

        void reloadDeviceList(bool advanced)
        {
            int num = MIDIGetNumberOfSources();
            m_clientFilter = !advanced;
            m_inputDevices.clear();
            m_inputDevices << MIDIConnection();
            for (int i = 0; i < num; ++i) {
                bool excluded = false;
                MIDIEndpointRef dest = MIDIGetSource( i );
                if (dest != 0) {
                    QString name = getEndpointName(dest);
                    if ( m_clientFilter &&
                         name.contains(QStringLiteral("IAC"), Qt::CaseSensitive) )
                        continue;
                    if ( name.contains(m_publicName))
                        continue;
                    for (const QString& n : m_excludedNames) {
                        if (name.contains(n)) {
                            excluded = true;
                            break;
                        }
                    }
                    if (!excluded) {
                        m_inputDevices << MIDIConnection(name, i);
                    }
                }
            }
            if (!m_currentInput.first.isEmpty() && m_source != 0 &&
                !m_inputDevices.contains(m_currentInput)) {
                m_currentInput = MIDIConnection();
                m_source = 0;
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

        void open(const MIDIConnection& conn)
        {
            OSStatus result = noErr;
            m_diagnostics.clear();
            m_status = false;
            m_source = MIDIGetSource( conn.second.toInt() );
            result = MIDIPortConnectSource( m_port, m_source, nullptr );
            if (result != noErr) {
                registerStatus("MIDIPortConnectSource()", result);
                return;
            }
            m_currentInput = conn;
            m_status = (result == noErr);
            return;
        }

        void close()
        {
            OSStatus result = noErr;
            m_status = false;
            m_diagnostics.clear();
            if (m_source != 0) {
                result = MIDIPortDisconnectSource(m_port, m_source);
                if (result != noErr) {
                    registerStatus("MIDIPortDisconnectSource()", result);
                }
                m_source = 0;
                m_currentInput = MIDIConnection();
                m_status = (result == noErr);
            }
        }

        void emitSignals(QByteArray& packet)
        {
            int value = 0, j = 0;
            while(j < packet.length()) {
                int status = packet[j] & 0xf0;
                int channel = packet[j] & 0x0f;
                QByteArray data;
                switch (status) {
                case MIDI_STATUS_NOTEOFF:
                    if(m_out != nullptr && m_thruEnabled)
                        m_out->sendNoteOff(channel, packet[j+1], packet[j+2]);
                    emit m_inp->midiNoteOff(channel, packet[j+1], packet[j+2]);
                    j+=3;
                    break;
                case MIDI_STATUS_NOTEON:
                    if(m_out != nullptr && m_thruEnabled)
                        m_out->sendNoteOn(channel, packet[j+1], packet[j+2]);
                    emit m_inp->midiNoteOn(channel, packet[j+1], packet[j+2]);
                    j+=3;
                    break;
                case MIDI_STATUS_KEYPRESURE:
                    if(m_out != nullptr && m_thruEnabled)
                        m_out->sendKeyPressure(channel, packet[j+1], packet[j+2]);
                    emit m_inp->midiKeyPressure(channel, packet[j+1], packet[j+2]);
                    j+=3;
                    break;
                case MIDI_STATUS_CONTROLCHANGE:
                    if(m_out != nullptr && m_thruEnabled)
                        m_out->sendController(channel, packet[j+1], packet[j+2]);
                    emit m_inp->midiController(channel, packet[j+1], packet[j+2]);
                    j+=3;
                    break;
                case MIDI_STATUS_PROGRAMCHANGE:
                    if(m_out != nullptr && m_thruEnabled)
                        m_out->sendProgram(channel, packet[j+1]);
                    emit m_inp->midiProgram(channel, packet[j+1]);
                    j+=2;
                    break;
                case MIDI_STATUS_CHANNELPRESSURE:
                    if(m_out != nullptr && m_thruEnabled)
                        m_out->sendChannelPressure(channel, packet[j+1]);
                    emit m_inp->midiChannelPressure(channel, packet[j+1]);
                    j+=2;
                    break;
                case MIDI_STATUS_PITCHBEND:
                    value = (packet[j+1] + packet[j+2] * 0x80) - 8192;
                    if(m_out != nullptr && m_thruEnabled)
                        m_out->sendPitchBend(channel, value);
                    emit m_inp->midiPitchBend(channel, value);
                    j+=3;
                    break;
                case MIDI_STATUS_SYSEX:
                    if(m_out != nullptr && m_thruEnabled)
                        m_out->sendSysex(packet);
                    emit m_inp->midiSysex(packet);
                    j+=packet.length();
                    break;
                default:
                    registerStatus("invalid status", status);
                }
            }
        }
    };

    static void MacMIDIReadProc( const MIDIPacketList *pktlist, void *refCon, void *connRefCon )
    {
        Q_UNUSED(connRefCon)
        MacMIDIInputPrivate  *obj = nullptr;
        if (refCon != nullptr)
            obj = static_cast<MacMIDIInputPrivate*>(refCon);
        const MIDIPacket *packet = static_cast<const MIDIPacket *>(pktlist->packet);
        for (unsigned int i = 0; i < pktlist->numPackets; ++i) {
            if (obj != nullptr && packet != nullptr) {
                QByteArray data((const char *)packet->data, packet->length);
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
                QtConcurrent::run(obj, &MacMIDIInputPrivate::emitSignals, data);
#else
                obj->emitSignals(data);
#endif
            }
            packet = MIDIPacketNext(packet);
        }
    }

    MacMIDIInput::MacMIDIInput(QObject *parent) :
            MIDIInput(parent), d(new MacMIDIInputPrivate(this))
    {
    }

    MacMIDIInput::~MacMIDIInput()
    {
        delete d;
    }

    void MacMIDIInput::initialize(QSettings *settings)
    {
        Q_UNUSED(settings)
    }

    QString MacMIDIInput::backendName()
    {
        return QLatin1String("CoreMIDI");
    }

    QString MacMIDIInput::publicName()
    {
        return d->m_publicName;
    }

    void MacMIDIInput::setPublicName(QString name)
    {
        d->setPublicName(name);
    }

    QList<MIDIConnection> MacMIDIInput::connections(bool advanced)
    {
        d->reloadDeviceList(advanced);
        return d->m_inputDevices;
    }

    void MacMIDIInput::setExcludedConnections(QStringList conns)
    {
        d->m_excludedNames = conns;
    }

    void MacMIDIInput::open(const MIDIConnection& conn)
    {
        d->open(conn);
    }

    void MacMIDIInput::close()
    {
        d->close();
    }

    MIDIConnection MacMIDIInput::currentConnection()
    {
        return d->m_currentInput;
    }

    void MacMIDIInput::setMIDIThruDevice(MIDIOutput *device)
    {
        d->m_out = device;
    }

    void MacMIDIInput::enableMIDIThru(bool enable)
    {
        d->m_thruEnabled = enable;
    }

    bool MacMIDIInput::isEnabledMIDIThru()
    {
        return d->m_thruEnabled && d->m_out != 0;
    }

    QStringList MacMIDIInput::getDiagnostics()
    {
        return d->m_diagnostics;
    }

    bool MacMIDIInput::getStatus()
    {
        return d->m_status;
    }

}} // namespace drumstick::rt

