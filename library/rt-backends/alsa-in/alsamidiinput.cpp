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

#include <QMutexLocker>
#include <cmath>
#include "alsaclient.h"
#include "alsaport.h"
#include "alsaevent.h"
#include "alsamidiinput.h"
#include "rtmidioutput.h"

namespace drumstick {
namespace rt {

    static QString DEFAULT_PUBLIC_NAME(QLatin1String("MIDI In"));

    class ALSAMIDIInput::ALSAMIDIInputPrivate : public SequencerEventHandler
    {
    public:
        ALSAMIDIInput *m_inp;
        MIDIOutput *m_out;
        MidiClient *m_client;
        MidiPort *m_port;
        int m_portId;
        int m_clientId;
        bool m_thruEnabled;
        bool m_clientFilter;
        int m_runtimeAlsaNum;
        QString m_publicName;
        QString m_currentInput;
        QStringList m_inputDevices;
        QStringList m_excludedNames;
        QMutex m_openMutex;

        ALSAMIDIInputPrivate(ALSAMIDIInput *inp) :
            m_inp(inp),
            m_out(0),
            m_client(0),
            m_port(0),
            m_portId(-1),
            m_clientId(-1),
            m_thruEnabled(false),
            m_publicName(DEFAULT_PUBLIC_NAME)
        {
            m_runtimeAlsaNum = getRuntimeALSALibraryNumber();
            m_client = new MidiClient(m_inp);
            m_client->open();
            m_client->setClientName(m_publicName);
            m_port = m_client->createPort();
            m_port->setPortName("in");
            m_port->setCapability( SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE );
            m_port->setPortType( SND_SEQ_PORT_TYPE_APPLICATION | SND_SEQ_PORT_TYPE_MIDI_GENERIC );
            m_clientId = m_client->getClientId();
            m_portId = m_port->getPortId();
            m_port->setTimestamping(false);
            m_port->setTimestampReal(false);
            m_client->setHandler(this);
        }

        virtual ~ALSAMIDIInputPrivate()
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
            m_inputDevices.clear();
            QListIterator<PortInfo> it(m_client->getAvailableInputs());
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
                    if (name.startsWith(n)) {
                        excluded = true;
                        break;
                    }
                }
                if (!excluded)
                    m_inputDevices << name;
            }
            if (!m_currentInput.isEmpty() && !m_inputDevices.contains(m_currentInput)) {
                m_currentInput.clear();
            }
        }

        bool setSubscription(const QString &newDevice)
        {
            //qDebug() << Q_FUNC_INFO << newDevice;
            if (m_inputDevices.contains(newDevice)) {
                m_currentInput = newDevice;
                m_port->unsubscribeAll();
                m_port->subscribeFrom(newDevice);
                m_client->startSequencerInput();
                return true;
            }
            return false;
        }

        void clearSubscription()
        {
            if (!m_currentInput.isEmpty()) {
                m_client->stopSequencerInput();
                m_port->unsubscribeAll();
                m_currentInput.clear();
            }
        }

        void setPublicName(QString newName)
        {
            if (newName != m_publicName) {
                m_client->setClientName(newName);
                m_publicName = newName;
            }
        }

        void handleSequencerEvent(SequencerEvent* ev)
        {
            if ( !SequencerEvent::isConnectionChange(ev) )
                switch(ev->getSequencerType()) {
                case SND_SEQ_EVENT_NOTEOFF: {
                        const NoteOffEvent* n = static_cast<const NoteOffEvent*>(ev);
                        if(m_out != 0 && m_thruEnabled) {
                            m_out->sendNoteOff(n->getChannel(), n->getKey(), n->getVelocity());
                        }
                        emit m_inp->midiNoteOff(n->getChannel(), n->getKey(), n->getVelocity());
                    }
                    break;
                case SND_SEQ_EVENT_NOTEON: {
                        const NoteOnEvent* n = static_cast<const NoteOnEvent*>(ev);
                        if(m_out != 0 && m_thruEnabled) {
                            m_out->sendNoteOn(n->getChannel(), n->getKey(), n->getVelocity());
                        }
                        emit m_inp->midiNoteOn(n->getChannel(), n->getKey(), n->getVelocity());
                    }
                    break;
                case SND_SEQ_EVENT_KEYPRESS: {
                        const KeyPressEvent* n = static_cast<const KeyPressEvent*>(ev);
                        if(m_out != 0 && m_thruEnabled) {
                            m_out->sendKeyPressure(n->getChannel(), n->getKey(), n->getVelocity());
                        }
                        emit m_inp->midiKeyPressure(n->getChannel(), n->getKey(), n->getVelocity());
                    }
                    break;
                case SND_SEQ_EVENT_CONTROLLER:
                case SND_SEQ_EVENT_CONTROL14: {
                        const ControllerEvent* n = static_cast<const ControllerEvent*>(ev);
                        if(m_out != 0 && m_thruEnabled) {
                            m_out->sendController(n->getChannel(), n->getParam(), n->getValue());
                        }
                        emit m_inp->midiController(n->getChannel(), n->getParam(), n->getValue());
                    }
                    break;
                case SND_SEQ_EVENT_PGMCHANGE: {
                        const ProgramChangeEvent* p = static_cast<const ProgramChangeEvent*>(ev);
                        if(m_out != 0 && m_thruEnabled) {
                            m_out->sendProgram(p->getChannel(), p->getValue());
                        }
                        emit m_inp->midiProgram(p->getChannel(), p->getValue());
                    }
                    break;
                case SND_SEQ_EVENT_CHANPRESS: {
                        const ChanPressEvent* n = static_cast<const ChanPressEvent*>(ev);
                        if(m_out != 0 && m_thruEnabled) {
                            m_out->sendChannelPressure(n->getChannel(), n->getValue());
                        }
                        emit m_inp->midiChannelPressure(n->getChannel(), n->getValue());
                    }
                    break;
                case SND_SEQ_EVENT_PITCHBEND: {
                        const PitchBendEvent* n = static_cast<const PitchBendEvent*>(ev);
                        if(m_out != 0 && m_thruEnabled) {
                            m_out->sendPitchBend(n->getChannel(), n->getValue());
                        }
                        emit m_inp->midiPitchBend(n->getChannel(), n->getValue());
                    }
                    break;
                case SND_SEQ_EVENT_SYSEX: {
                        const SysExEvent* n = static_cast<const SysExEvent*>(ev);
                        QByteArray data(n->getData(), n->getLength());
                        if(m_out != 0 && m_thruEnabled) {
                            m_out->sendSysex(data);
                        }
                        emit m_inp->midiSysex(data);
                    }
                    break;
                case SND_SEQ_EVENT_SYSTEM: {
                        const SystemEvent* n = static_cast<const SystemEvent*>(ev);
                        int status = (int) n->getRaw8(0);
                        if(m_out != 0 && m_thruEnabled) {
                            m_out->sendSystemMsg(status);
                        }
                        if (status < 0xF7)
                            emit m_inp->midiSystemCommon(status);
                        else if (status > 0xF7)
                            emit m_inp->midiSystemRealtime(status);
                    }
                    break;
                default:
                    break;
                }
            delete ev;
        }
    };

    ALSAMIDIInput::ALSAMIDIInput(QObject *parent) : MIDIInput(parent),
        d(new ALSAMIDIInputPrivate(this))
    { }

    ALSAMIDIInput::~ALSAMIDIInput()
    {
        delete d;
    }

    void ALSAMIDIInput::initialize(QSettings* settings)
    {
        Q_UNUSED(settings)
    }

    QString ALSAMIDIInput::backendName()
    {
        return QLatin1String("ALSA");
    }

    QString ALSAMIDIInput::publicName()
    {
        return d->m_publicName;
    }

    void ALSAMIDIInput::setPublicName(QString name)
    {
        d->setPublicName(name);
    }

    QStringList ALSAMIDIInput::connections(bool advanced)
    {
        d->reloadDeviceList(advanced);
        return d->m_inputDevices;
    }

    void ALSAMIDIInput::setExcludedConnections(QStringList conns)
    {
        d->m_excludedNames = conns;
    }

    void ALSAMIDIInput::open(QString name)
    {
        d->setSubscription(name);
    }

    void ALSAMIDIInput::close()
    {
        d->clearSubscription();
    }

    QString ALSAMIDIInput::currentConnection()
    {
        return d->m_currentInput;
    }

    void ALSAMIDIInput::setMIDIThruDevice(MIDIOutput *device)
    {
        d->m_out = device;
    }

    void ALSAMIDIInput::enableMIDIThru(bool enable)
    {
        d->m_thruEnabled = enable;
    }

    bool ALSAMIDIInput::isEnabledMIDIThru()
    {
        return d->m_thruEnabled && (d->m_out != 0);
    }

}}
