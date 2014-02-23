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

#include <CoreFoundation/CoreFoundation.h>
#include <CoreMIDI/MIDIServices.h>

namespace drumstick {
namespace rt {

    static CFStringRef DEFAULT_PUBLIC_NAME CFSTR("MIDI Out");

#if QT_VERSION < QT_VERSION_CHECK(5,2,0)
    static QString CFStringToQString(CFStringRef str)
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
#endif

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
            QString s =QString::fromCFString(str);
            CFRelease (str);
            str = 0;
            if (!result.startsWith(s, Qt::CaseInsensitive) )
                result = (s + ' ' + result).trimmed();
        }
        return result;
    }

    class MacMIDIOutput::MacMIDIOutputPrivate {
    public:
        MIDIClientRef m_client;
        MIDIPortRef m_outPort;
        MIDIEndpointRef m_endpoint;
        MIDIEndpointRef m_destination;
        bool m_clientFilter;

        QString m_currentOutput;
        QString m_publicName;
        QStringList m_excludedNames;
        QStringList m_outputDevices;

        MacMIDIOutputPrivate():
            m_client(0),
            m_outPort(0),
            m_endpoint(0),
            m_destination(0),
            m_clientFilter(true),
            m_publicName(QString::fromCFString(DEFAULT_PUBLIC_NAME))
        {
            OSStatus result;
            result = MIDIClientCreate(DEFAULT_PUBLIC_NAME, NULL, NULL, &m_client);
            if ( result != noErr )
                return;

            result = MIDISourceCreate(m_client, DEFAULT_PUBLIC_NAME, &m_endpoint);
            if ( result != noErr )
                return;
            reloadDeviceList(true);
        }

        virtual ~MacMIDIOutputPrivate()
        {
            OSStatus result = noErr;
            if (m_client != 0)
              result = MIDIClientDispose(m_client);
            if (result != noErr)
                qDebug() << "MIDIClientDispose() error:" << result;
        }

        void setPublicName(QString name)
        {
            if (m_publicName != name) {
                m_publicName = name;
                //MIDIObjectSetStringProperty(m_client, kMIDIPropertyName, QString::toCFString(name));
            }
        }

        void reloadDeviceList(bool advanced)
        {
            int num = MIDIGetNumberOfDestinations();
            m_clientFilter = advanced;
            m_outputDevices.clear();
            for (int i = 0; i < num; ++i) {
                QString result;
                MIDIEndpointRef dest = MIDIGetDestination( i );
                if (dest != 0) {
                    QString name = getEndpointName(dest);
                    if ( m_clientFilter &&
                         name.contains(QLatin1String("IAC"), Qt::CaseSensitive) )
                        continue;
                    m_outputDevices << name;
                }
            }
            if (!m_currentOutput.isEmpty() &&
                !m_outputDevices.contains(m_currentOutput)) {
                m_currentOutput.clear();
                m_destination = 0;
            }
        }

        void sendEvents( const MIDIPacketList* events )
        {
            MIDIReceived(m_endpoint, events);
            if (m_destination != 0)
                MIDISend(m_outPort, m_destination, events);
        }

        bool open(const QString &newOutputDevice)
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
            OSStatus result = MIDIOutputPortCreate(m_client, DEFAULT_PUBLIC_NAME, &m_outPort);
            if (result != noErr) {
                qDebug() << "MIDIOutputPortCreate() error:" << result;
                return false;
            }
            m_destination = MIDIGetDestination( index );
            m_currentOutput = newOutputDevice;
            return true;
        }

        void close()
        {
            MIDIPortDispose( m_outPort );
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

    QString MacMIDIOutput::backendName()
    {
        return QLatin1Literal("Mac OSX CoreMIDI");
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
        data[1] = val % 0x80; // LSB
        data[2] = val / 0x80; // MSB
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

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
    Q_EXPORT_PLUGIN2(drumstick_rt_mac_out, MacMIDIOutput)
#endif
}}
