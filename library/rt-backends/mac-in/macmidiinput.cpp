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

#include "macmidiinput.h"
#include "rtmidioutput.h"
#include "maccommon.h"

#include <QDebug>
#include <QStringList>
#include <QMutex>
#include <QTextCodec>
#include <QObject>

#include <CoreFoundation/CoreFoundation.h>
#include <CoreMIDI/CoreMIDI.h>

namespace drumstick {
namespace rt {

    static CFStringRef DEFAULT_PUBLIC_NAME CFSTR("MIDI In");

    void MacMIDIReadProc( const MIDIPacketList *pktlist,
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
        QString m_currentInput;
        QStringList m_excludedNames;
        QStringList m_inputDevices;

        MacMIDIInputPrivate(MacMIDIInput *inp) :
            m_inp(inp),
            m_out(0),
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

        void internalCreate(CFStringRef name)
        {
            OSStatus result = noErr;
            result = MIDIClientCreate( name , NULL, NULL, &m_client );
            if (result != noErr) {
                qDebug() << "MIDIClientCreate() err:" << result;
                return;
            }
            result = MIDIDestinationCreate ( m_client, name, MacMIDIReadProc, (void*) this, &m_endpoint );
            if (result != noErr) {
                qDebug() << "MIDIDestinationCreate() err:" << result;
                return;
            }
            result = MIDIInputPortCreate( m_client, name, MacMIDIReadProc, (void *) this,  &m_port );
            if (result != noErr) {
                qDebug() << "MIDIInputPortCreate() error:" << result;
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
                    qDebug() << "MIDIPortDispose() error:" << result;
                    m_port = 0;
                }
            }
            if (m_endpoint != 0) {
                result = MIDIEndpointDispose(m_endpoint);
                if (result != noErr) {
                    qDebug() << "MIDIEndpointDispose() err:" << result;
                    m_endpoint = 0;
                }
            }
            if (m_client != 0) {
                result = MIDIClientDispose(m_client);
                if (result != noErr) {
                    qDebug() << "MIDIClientDispose() err:" << result;
                    m_client = 0;
                }
            }
        }

        void reloadDeviceList(bool advanced)
        {
            int num = MIDIGetNumberOfSources();
            m_clientFilter = !advanced;
            m_inputDevices.clear();
            for (int i = 0; i < num; ++i) {
                bool excluded = false;
                MIDIEndpointRef dest = MIDIGetSource( i );
                if (dest != 0) {
                    QString name = getEndpointName(dest);
                    if ( m_clientFilter &&
                         name.contains(QLatin1String("IAC"), Qt::CaseSensitive) )
                        continue;
                    if ( name.contains(m_publicName))
                        continue;
                    foreach( const QString& n, m_excludedNames) {
                        if (name.contains(n)) {
                            excluded = true;
                            break;
                        }
                    }
                    if (!excluded)
                        m_inputDevices << name;
                }
            }
            if (!m_currentInput.isEmpty() && m_source != 0 &&
                !m_inputDevices.contains(m_currentInput)) {
                m_currentInput.clear();
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

        void open(QString name)
        {
            int index = -1;
            OSStatus result = noErr;
            QStringList allInputDevices;
            int num = MIDIGetNumberOfSources();
            for (int i = 0; i < num; ++i) {
                MIDIEndpointRef dest = MIDIGetSource( i );
                if (dest != 0)
                   allInputDevices << getEndpointName( dest );
            }
            index = allInputDevices.indexOf(name);
            if (index < 0)
                return;
            m_source = MIDIGetSource( index );
            result = MIDIPortConnectSource( m_port, m_source, NULL );
            if (result != noErr) {
                qDebug() << "MIDIPortConnectSource() error:" << result;
                return;
            }
            m_currentInput = name;
            return;
        }

        void close()
        {
            OSStatus result = noErr;
            if (m_source != 0) {
                result = MIDIPortDisconnectSource(m_port, m_source);
                if (result != noErr)
                    qDebug() << "MIDIPortDisconnectSource() error:" << result;
                m_source = 0;
                m_currentInput.clear();
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
                if(m_out != 0 && m_thruEnabled)
                    m_out->sendNoteOff(channel, packet->data[1], packet->data[2]);
                emit m_inp->midiNoteOff(channel, packet->data[1], packet->data[2]);
                break;
            case MIDI_STATUS_NOTEON:
                if(m_out != 0 && m_thruEnabled)
                    m_out->sendNoteOn(channel, packet->data[1], packet->data[2]);
                emit m_inp->midiNoteOn(channel, packet->data[1], packet->data[2]);
                break;
            case MIDI_STATUS_KEYPRESURE:
                if(m_out != 0 && m_thruEnabled)
                    m_out->sendKeyPressure(channel, packet->data[1], packet->data[2]);
                emit m_inp->midiKeyPressure(channel, packet->data[1], packet->data[2]);
                break;
            case MIDI_STATUS_CONTROLCHANGE:
                if(m_out != 0 && m_thruEnabled)
                    m_out->sendController(channel, packet->data[1], packet->data[2]);
                emit m_inp->midiController(channel, packet->data[1], packet->data[2]);
                break;
            case MIDI_STATUS_PROGRAMCHANGE:
                if(m_out != 0 && m_thruEnabled)
                    m_out->sendProgram(channel, packet->data[1]);
                emit m_inp->midiProgram(channel, packet->data[1]);
                break;
            case MIDI_STATUS_CHANNELPRESSURE:
                if(m_out != 0 && m_thruEnabled)
                    m_out->sendChannelPressure(channel, packet->data[1]);
                emit m_inp->midiChannelPressure(channel, packet->data[1]);
                break;
            case MIDI_STATUS_PITCHBEND:
                value = (packet->data[1] + packet->data[2] * 0x80) - 8192;
                if(m_out != 0 && m_thruEnabled)
                    m_out->sendPitchBend(channel, value);
                emit m_inp->midiPitchBend(channel, value);
                break;
            case MIDI_STATUS_SYSEX:
                data = QByteArray((const char *)packet->data, packet->length);
                if(m_out != 0 && m_thruEnabled)
                    m_out->sendSysex(data);
                emit m_inp->midiSysex(data);
                break;
            default:
                qDebug() << "status?" << status;
            }
        }

    };

    void MacMIDIReadProc( const MIDIPacketList *pktlist,
                          void *refCon, void *connRefCon )
    {
        Q_UNUSED(connRefCon)
        MacMIDIInputPrivate  *obj = NULL;
        if (refCon != NULL)
            obj = static_cast<MacMIDIInputPrivate*>(refCon);

        MIDIPacket *packet = (MIDIPacket *)pktlist->packet;
        for (unsigned int i = 0; i < pktlist->numPackets; ++i) {
            if (obj != NULL)
               obj->emitSignals(packet);
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
        return QLatin1Literal("CoreMIDI");
    }

    QString MacMIDIInput::publicName()
    {
        return d->m_publicName;
    }

    void MacMIDIInput::setPublicName(QString name)
    {
        d->setPublicName(name);
    }

    QStringList MacMIDIInput::connections(bool advanced)
    {
        d->reloadDeviceList(advanced);
        return d->m_inputDevices;
    }

    void MacMIDIInput::setExcludedConnections(QStringList conns)
    {
        d->m_excludedNames = conns;
    }

    void MacMIDIInput::open(QString name)
    {
        d->open(name);
    }

    void MacMIDIInput::close()
    {
        d->close();
    }

    QString MacMIDIInput::currentConnection()
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

}} // namespace drumstick::rt

