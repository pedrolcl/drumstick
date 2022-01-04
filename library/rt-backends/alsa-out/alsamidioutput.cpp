/*
    Drumstick RT Backend using the ALSA Sequencer
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

#include "alsamidioutput.h"
#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include <QString>
#include <QStringList>
#include <drumstick/alsaclient.h>
#include <drumstick/alsaevent.h>
#include <drumstick/alsaport.h>

namespace drumstick {
namespace rt {

    using namespace ALSA;

    const QString ALSAMIDIOutput::DEFAULT_PUBLIC_NAME = QStringLiteral("MIDI Out");

    class ALSAMIDIOutput::ALSAMIDIOutputPrivate {
    public:
        ALSAMIDIOutput *m_out;
        MidiClient *m_client;
        MidiPort *m_port;
        int m_portId;
        bool m_clientFilter;
        int m_runtimeAlsaNum;
        QString m_publicName;
        MIDIConnection m_currentOutput;
        QList<MIDIConnection> m_outputDevices;
        QStringList m_excludedNames;
        QMutex m_outMutex;
        bool m_initialized;
        bool m_status;
        QStringList m_diagnostics;

        explicit ALSAMIDIOutputPrivate(ALSAMIDIOutput *q):
            m_out(q),
            m_client(nullptr),
            m_port(nullptr),
            m_portId(0),
            m_clientFilter(true),
            m_runtimeAlsaNum(0),
            m_publicName(DEFAULT_PUBLIC_NAME),
            m_initialized(false),
            m_status(false)
        {
            m_runtimeAlsaNum = getRuntimeALSALibraryNumber();
            m_diagnostics.clear();
        }

        ~ALSAMIDIOutputPrivate()
        {
            if (m_initialized) {
                clearSubscription();
                uninitialize();
            }
        }

        void initialize()
        {
            //qDebug() << Q_FUNC_INFO << m_initialized;
            if (!m_initialized) {
                m_client = new MidiClient(m_out);
                m_client->open();
                m_client->setClientName(m_publicName);
                m_port = m_client->createPort();
                m_port->setPortName("out");
                m_port->setCapability( SND_SEQ_PORT_CAP_READ  | SND_SEQ_PORT_CAP_SUBS_READ );
                m_port->setPortType( SND_SEQ_PORT_TYPE_APPLICATION | SND_SEQ_PORT_TYPE_MIDI_GENERIC );
                m_portId = m_port->getPortId();
                m_initialized = true;
                m_status = true;
                m_diagnostics.clear();
            }
        }

        void uninitialize()
        {
            //qDebug() << Q_FUNC_INFO << m_initialized;
            if (m_initialized) {
                if (m_port != nullptr) {
                    m_port->detach();
                    delete m_port;
                    m_port = nullptr;
                }
                if (m_client != nullptr) {
                    m_client->close();
                    delete m_client;
                    m_client = nullptr;
                }
                m_initialized = false;
                m_status = false;
                m_diagnostics.clear();
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
            QStringList clientNames;
            QMap<QString,int> namesMap;
            if (!m_initialized) {
                initialize();
            }
            auto outputs = m_client->getAvailableOutputs();
            m_clientFilter = !advanced;
            m_outputDevices.clear();
            for (const PortInfo& p : qAsConst(outputs)) {
                QString name = p.getClientName();
                clientNames << name;
                if (namesMap.contains(name)) {
                    namesMap[name]++;
                } else {
                    namesMap.insert(name, 1);
                }
            }
            for (PortInfo& p : outputs) {
                bool excluded = false;
                QString name = p.getClientName();
                if (m_clientFilter && clientIsAdvanced(p.getClient()))
                    continue;
                if ( m_clientFilter && name.startsWith(QStringLiteral("Virtual Raw MIDI")) )
                    continue;
                if ( name.startsWith(m_publicName) )
                    continue;
                for (const QString& n : qAsConst(m_excludedNames)) {
                    if ( name.startsWith(n) ) {
                        excluded = true;
                        break;
                    }
                }
                if (!excluded) {
                    int k = clientNames.count(name) + 1;
                    QString addr = QString("%1:%2").arg(p.getClient()).arg(p.getPort());
                    if (k > 2) {
                        m_outputDevices << MIDIConnection(QString("%1 (%2)").arg(name).arg(k - namesMap[name]--), addr);
                    } else {
                        m_outputDevices << MIDIConnection(name, addr);
                    }
                }
            }
            if (!m_currentOutput.first.isEmpty() &&
                !m_outputDevices.contains(m_currentOutput)) {
                m_currentOutput = MIDIConnection();
            }
        }

        bool setSubscription(const MIDIConnection &newOutputDevice)
        {
            if (!m_initialized) {
                initialize();
            }
            if (m_outputDevices.contains(newOutputDevice)) {
                m_currentOutput = newOutputDevice;
                m_port->unsubscribeAll();
                m_port->subscribeTo(newOutputDevice.second.toString());
                return true;
            }
            return false;
        }

        void clearSubscription()
        {
            if (!m_currentOutput.first.isEmpty() && m_initialized) {
                m_port->unsubscribeAll();
                m_currentOutput = MIDIConnection();
            }
        }

        void sendEvent(SequencerEvent *ev)
        {
            if (!m_initialized) {
                initialize();
            }
            QMutexLocker locker(&m_outMutex);
            ev->setSource(m_portId);
            ev->setSubscribers();
            ev->setDirect();
            m_client->outputDirect(ev);
        }

        void setPublicName(QString newName)
        {
            if (newName != m_publicName) {
                m_publicName = newName;
                if (m_initialized) {
                    m_client->setClientName(newName);
                }
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
        d->initialize();
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

    QStringList ALSAMIDIOutput::getDiagnostics()
    {
        return d->m_diagnostics;
    }

    bool ALSAMIDIOutput::getStatus()
    {
        return d->m_status;
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

    QList<MIDIConnection> ALSAMIDIOutput::connections(bool advanced)
    {
        d->reloadDeviceList(advanced);
        return d->m_outputDevices;
    }

    void ALSAMIDIOutput::setExcludedConnections(QStringList conns)
    {
        d->m_excludedNames = conns;
    }

    MIDIConnection ALSAMIDIOutput::currentConnection()
    {
        return d->m_currentOutput;
    }

    void ALSAMIDIOutput::open(const MIDIConnection& name)
    {
        bool b = d->setSubscription(name);
        if (!b) {
            d->m_diagnostics << "failed subscription to " + name.first;
        }
    }

    void ALSAMIDIOutput::close()
    {
        d->clearSubscription();
        d->uninitialize();
    }

} // namespace rt
} // namespace drumstick

