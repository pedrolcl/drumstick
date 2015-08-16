/*
    Drumstick RT Backend using the ALSA Sequencer
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

#include <QString>
#include <QStringList>
#include <QMutex>
#include <QMutexLocker>
#include <qmath.h>
#include "alsaclient.h"
#include "alsaport.h"
#include "alsaevent.h"
#include "alsamidioutput.h"

namespace drumstick {
namespace rt {

    static QString DEFAULT_PUBLIC_NAME(QLatin1String("MIDI Out"));

    class ALSAMIDIOutput::ALSAMIDIOutputPrivate {
    public:
        ALSAMIDIOutput *m_out;
        MidiClient *m_client;
        MidiPort *m_port;
        int m_portId;
        bool m_clientFilter;
        int m_runtimeAlsaNum;
        QString m_publicName;
        QString m_currentOutput;
        QStringList m_outputDevices;
        QStringList m_excludedNames;
        QMutex m_outMutex;

        ALSAMIDIOutputPrivate(ALSAMIDIOutput *q):
            m_out(q),
            m_client(0),
            m_port(0),
            m_portId(0),
            m_clientFilter(true),
            m_runtimeAlsaNum(0),
            m_publicName(DEFAULT_PUBLIC_NAME)
        {
            m_runtimeAlsaNum = getRuntimeALSALibraryNumber();
            m_client = new MidiClient(m_out);
            m_client->open();
            m_client->setClientName(m_publicName);
            m_port = m_client->createPort();
            m_port->setPortName("out");
            m_port->setCapability( SND_SEQ_PORT_CAP_READ  | SND_SEQ_PORT_CAP_SUBS_READ );
            m_port->setPortType( SND_SEQ_PORT_TYPE_APPLICATION | SND_SEQ_PORT_TYPE_MIDI_GENERIC );
            m_portId = m_port->getPortId();
        }

        ~ALSAMIDIOutputPrivate()
        {
            if (m_client != NULL) {
                clearSubscription();
                if (m_port != NULL)
                    m_port->detach();
                m_client->close();
                delete m_client;
            }
        }

        bool clientIsAdvanced(int clientId)
        {
            // asking for runtime version instead of SND_LIB_VERSION
            if (m_runtimeAlsaNum < 0x01000B)
                // ALSA <= 1.0.10
                return (clientId < 64);
            else
                // ALSA >= 1.0.11
                return (clientId < 16);
        }

        void reloadDeviceList(bool advanced)
        {
            m_clientFilter = !advanced;
            m_outputDevices.clear();
            QListIterator<PortInfo> it(m_client->getAvailableOutputs());
            while(it.hasNext()) {
                bool excluded = false;
                PortInfo p = it.next();
                QString name = QString("%1:%2").arg(p.getClientName()).arg(p.getPort());
                if (m_clientFilter && clientIsAdvanced(p.getClient()))
                    continue;
                if ( m_clientFilter && name.startsWith(QLatin1String("Virtual Raw MIDI")) )
                    continue;
                if ( name.startsWith(m_publicName) )
                    continue;
                foreach(const QString& n, m_excludedNames) {
                    if ( name.startsWith(n) ) {
                        excluded = true;
                        break;
                    }
                }
                if (!excluded)
                    m_outputDevices << name;
            }
            if (!m_currentOutput.isEmpty() &&
                !m_outputDevices.contains(m_currentOutput)) {
                m_currentOutput.clear();
            }
        }

        bool setSubscription(const QString &newOutputDevice)
        {
            //qDebug() << Q_FUNC_INFO << newOutputDevice;
            if (m_outputDevices.contains(newOutputDevice)) {
                m_currentOutput = newOutputDevice;
                m_port->unsubscribeAll();
                m_port->subscribeTo(newOutputDevice);
                return true;
            }
            return false;
        }

        void clearSubscription()
        {
            if (!m_currentOutput.isEmpty()) {
                m_port->unsubscribeAll();
                m_currentOutput.clear();
            }
        }

        void sendEvent(SequencerEvent *ev)
        {
            QMutexLocker locker(&m_outMutex);
            ev->setSource(m_portId);
            ev->setSubscribers();
            ev->setDirect();
            m_client->outputDirect(ev);
        }

        void setPublicName(QString newName)
        {
            if (newName != m_publicName) {
                m_client->setClientName(newName);
                m_publicName = newName;
            }
        }

    };

    ALSAMIDIOutput::ALSAMIDIOutput(QObject *parent) : MIDIOutput(parent),
        d(new ALSAMIDIOutputPrivate(this))
    { }

    ALSAMIDIOutput::~ALSAMIDIOutput()
    {
        delete d;
    }

    void ALSAMIDIOutput::initialize(QSettings* settings)
    {
        Q_UNUSED(settings)
    }

    /* SLOTS */

    void ALSAMIDIOutput::sendNoteOn(int chan, int note, int vel)
    {
        NoteOnEvent ev(chan, note, vel);
        d->sendEvent(&ev);
    }

    void ALSAMIDIOutput::sendNoteOff(int chan, int note, int vel)
    {
        NoteOffEvent ev(chan, note, vel);
        d->sendEvent(&ev);
    }

    void ALSAMIDIOutput::sendController(int chan, int control, int value)
    {
        ControllerEvent ev(chan, control, value);
        d->sendEvent(&ev);
    }

    void ALSAMIDIOutput::sendKeyPressure(int chan, int note, int value)
    {
        KeyPressEvent ev(chan, note, value);
        d->sendEvent(&ev);
    }

    void ALSAMIDIOutput::sendProgram(int chan, int program)
    {
        ProgramChangeEvent ev(chan, program);
        d->sendEvent(&ev);
    }

    void ALSAMIDIOutput::sendChannelPressure(int chan, int value)
    {
        ChanPressEvent ev(chan, value);
        d->sendEvent(&ev);
    }

    void ALSAMIDIOutput::sendPitchBend(int chan, int value)
    {
        PitchBendEvent ev(chan, value);
        d->sendEvent(&ev);
    }

    void ALSAMIDIOutput::sendSysex(const QByteArray& data)
    {
        SysExEvent ev(data);
        d->sendEvent(&ev);
    }

    void ALSAMIDIOutput::sendSystemMsg(const int status)
    {
        SystemEvent ev(status);
        d->sendEvent(&ev);
    }

    QString ALSAMIDIOutput::backendName()
    {
        return "ALSA";
    }

    QString ALSAMIDIOutput::publicName()
    {
        return d->m_publicName;
    }

    void ALSAMIDIOutput::setPublicName(QString name)
    {
        d->setPublicName(name);
    }

    QStringList ALSAMIDIOutput::connections(bool advanced)
    {
        d->reloadDeviceList(advanced);
        return d->m_outputDevices;
    }

    void ALSAMIDIOutput::setExcludedConnections(QStringList conns)
    {
        d->m_excludedNames = conns;
    }

    QString ALSAMIDIOutput::currentConnection()
    {
        return d->m_currentOutput;
    }

    void ALSAMIDIOutput::open(QString name)
    {
        d->setSubscription(name);
    }

    void ALSAMIDIOutput::close()
    {
        d->clearSubscription();
    }

}}

