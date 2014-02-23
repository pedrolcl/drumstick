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

#include <CoreFoundation/CoreFoundation.h>
//#include <CoreServices/CoreServices.h>
//#include <AudioToolbox/AudioToolbox.h>
//#include <AudioToolbox/MusicPlayer.h>
#include <CoreMIDI/CoreMIDI.h>
#include <CoreMIDI/MIDIServices.h>

namespace drumstick {
namespace rt {

    static CFStringRef DEFAULT_PUBLIC_NAME CFSTR("MIDI In");

    void MacMIDIReadProc( const MIDIPacketList *pktlist,
                          void *refCon, void *connRefCon );

    static QString getEndpointName(MIDIEndpointRef endpoint)
    {
        QString result;
        CFStringRef str = 0;
        MIDIObjectGetStringProperty (endpoint, kMIDIPropertyName, &str);
        if (str != 0) {
            result = QString::fromCFString(str);
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
                result = QString::fromCFString(str);
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
            QString s = QString::fromCFString(str);
            CFRelease (str);
            str = 0;
            if (!result.startsWith(s, Qt::CaseInsensitive) )
                result = (s + ' ' + result).trimmed();
        }
        return result;
    }

    class MacMIDIInputPrivate {
    public:
        MacMIDIInput *m_inp;
        MIDIOutput *m_out;
        MIDIClientRef m_client;
        MIDIEndpointRef m_endpoint;
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
            m_endpoint(0),
            m_thruEnabled(false),
            m_clientFilter(true),
            m_publicName(QString::fromCFString(DEFAULT_PUBLIC_NAME))
        {
            OSStatus result = MIDIClientCreate( DEFAULT_PUBLIC_NAME, NULL, NULL, &m_client);
            if (result != noErr)
                qDebug() << "MIDIClientCreate() err:" << result;
            result = MIDIDestinationCreate ( m_client, DEFAULT_PUBLIC_NAME, MacMIDIReadProc,
                        (void*) this, &m_endpoint );
            if (result != noErr)
                qDebug() << "MIDIDestinationCreate() err:" << result;
        }

        virtual ~MacMIDIInputPrivate()
        {
            OSStatus result;
            if (m_endpoint != 0) {
                result = MIDIEndpointDispose(m_endpoint);
                if (result != noErr)
                    qDebug() << "MIDIEndpointDispose() err:" << result;
            }
            if (m_client != 0)
            {
                result = MIDIClientDispose(m_client);
                if (result != noErr)
                    qDebug() << "MIDIClientDispose() err:" << result;
            }
        }

        void reloadDeviceList(bool advanced)
        {
            int num = MIDIGetNumberOfSources();
            m_clientFilter = advanced;
            m_inputDevices.clear();
            for (int i = 0; i < num; ++i) {
                QString result;
                MIDIEndpointRef dest = MIDIGetSource( i );
                if (dest != 0) {
                    QString name = getEndpointName(dest);
                    if ( m_clientFilter &&
                         name.contains(QLatin1String("IAC"), Qt::CaseSensitive) )
                        continue;
                    m_inputDevices << name;
                }
            }
            if (!m_currentInput.isEmpty() &&
                !m_inputDevices.contains(m_currentInput)) {
                m_currentInput.clear();
                m_endpoint = 0;
            }
        }

        void setPublicName(QString name)
        {

        }

        void open(QString name)
        {

        }

        void close()
        {
            m_endpoint = 0;
            m_currentInput.clear();
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

    QString MacMIDIInput::backendName()
    {
        return QLatin1Literal("Mac OSX CoreMIDI");
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

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
    Q_EXPORT_PLUGIN2(drumstick_rt_mac_in, MacMIDIInput)
#endif
}} // namespace drumstick::rt

//#include "macmidiobject.moc"
