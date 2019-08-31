/*
    Drumstick RT Mac OSX Backend
    Copyright (C) 2009-2019 Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <QDebug>
#include <QSettings>
#include "macsynth.h"

#include <CoreServices/CoreServices.h>
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h>

#define PRETTY_NAME "DLS Synth"

namespace drumstick {
namespace rt {

    class MacSynthOutput::MacSynthOutputPrivate
    {
    private:
        AUGraph m_graph;
        AudioUnit m_synthUnit;
        QString m_connection;
        QString m_soundfont_dls;
        bool m_default_dls;
        bool m_reverb_dls;

    public:
        explicit MacSynthOutputPrivate():
            m_graph(nullptr),
            m_synthUnit(nullptr)
        {
            //qDebug() << Q_FUNC_INFO;
            m_connection.clear();
        }

        ~MacSynthOutputPrivate()
        {
            //qDebug() << Q_FUNC_INFO;
            stop();
        }

        bool useDefaultDls() const
        {
            return m_default_dls;
        }

        void setDefaultDlsFlag(const bool f)
        {
            if (f != m_default_dls) {
                m_default_dls = f;
            }
        }

        bool useReverb() const
        {
            return m_reverb_dls;
        }

        void setReverbFlag(const bool f)
        {
            if (f != m_reverb_dls) {
                m_reverb_dls = f;
            }
        }

        QString soundFontDls() const
        {
            return m_soundfont_dls;
        }

        void setSoundFontDls(const QString& sf)
        {
            if (sf != m_soundfont_dls) {
                m_soundfont_dls = sf;
            }
        }

        QString currentConnection()
        {
            return m_connection;
        }

        void start ()
        {
            OSStatus result;
            AudioComponentDescription cd;
            UInt32 usesReverb;
            AUNode synthNode = 0;
            AUNode outputNode = 0;
            AUNode limiterNode = 0;

            //qDebug() << Q_FUNC_INFO;
            if (m_graph == nullptr) {
                cd.componentManufacturer = kAudioUnitManufacturer_Apple;
                cd.componentFlags = 0;
                cd.componentFlagsMask = 0;

                result = NewAUGraph (&m_graph);
                registerStatus("NewAUGraph", result);
                if (result != noErr)
                    return;

                cd.componentType = kAudioUnitType_MusicDevice;
                cd.componentSubType = kAudioUnitSubType_DLSSynth;
                result = AUGraphAddNode (m_graph, &cd, &synthNode);
                registerStatus( "AUGraphAddNode", result);

                cd.componentType = kAudioUnitType_Effect;
                cd.componentSubType = kAudioUnitSubType_PeakLimiter;
                result = AUGraphAddNode(m_graph, &cd, &limiterNode);
                registerStatus( "AUGraphAddNode", result);

                cd.componentType = kAudioUnitType_Output;
                cd.componentSubType = kAudioUnitSubType_DefaultOutput;
                result = AUGraphAddNode (m_graph, &cd, &outputNode);
                registerStatus( "AUGraphAddNode", result);

                result = AUGraphOpen (m_graph);
                registerStatus( "AUGraphOpen", result);
                if (result != noErr)
                    return;

                result = AUGraphConnectNodeInput (m_graph, synthNode, 0, limiterNode, 0);
                registerStatus( "AUGraphConnectNodeInput", result);

                result = AUGraphConnectNodeInput (m_graph, limiterNode, 0, outputNode, 0);
                registerStatus( "AUGraphConnectNodeInput", result);

                result = AUGraphNodeInfo (m_graph, synthNode, nullptr, &m_synthUnit);
                registerStatus( "AUGraphNodeInfo", result);

                if (!m_default_dls && !m_soundfont_dls.isEmpty()) {
                    QByteArray utf8file = m_soundfont_dls.toUtf8();
                    CFURLRef url = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault,
                                                                           reinterpret_cast<const UInt8*>(utf8file.data()),
                                                                           utf8file.length(), false);
                    if (url) {
                        // kMusicDeviceProperty_SoundBankURL since 10.5
                        result = AudioUnitSetProperty(m_synthUnit, kMusicDeviceProperty_SoundBankURL, kAudioUnitScope_Global,
                                                      0, &url, sizeof(url));
                        registerStatus( "AudioUnitSetProperty(SoundBankURL)", result);
                        CFRelease(url);
                    }
                }

                usesReverb = (m_reverb_dls ? 1 : 0);
                //qDebug() << "usesReverb =" << usesReverb;
                result = AudioUnitSetProperty ( m_synthUnit,
                    kMusicDeviceProperty_UsesInternalReverb, kAudioUnitScope_Global,
                    0, &usesReverb, sizeof (usesReverb) );
                registerStatus( "AudioUnitSetProperty(UsesInternalReverb)", result);

                result = AUGraphInitialize (m_graph);
                registerStatus( "AUGraphInitialize", result);
                if (result != noErr) {
                    return;
                }

                for (uint i = 0; i < 16; ++i) {
                    result = MusicDeviceMIDIEvent(m_synthUnit, MIDI_STATUS_CONTROLCHANGE+i, MIDI_CTL_MSB_MAIN_VOLUME,100,0);
                    registerStatus( "MusicDeviceMIDIEvent", result );
                    result = MusicDeviceMIDIEvent(m_synthUnit, MIDI_STATUS_CONTROLCHANGE+i, MIDI_CTL_REVERB_SEND,100,0);
                    registerStatus( "MusicDeviceMIDIEvent", result );
                    result = MusicDeviceMIDIEvent(m_synthUnit, MIDI_STATUS_CONTROLCHANGE+i, MIDI_CTL_MSB_BANK_SELECT,0,0);
                    registerStatus( "MusicDeviceMIDIEvent", result );
                    result = MusicDeviceMIDIEvent(m_synthUnit, MIDI_STATUS_CONTROLCHANGE+i, MIDI_CTL_LSB_BANK_SELECT,0,0);
                    registerStatus( "MusicDeviceMIDIEvent", result );
                    result = MusicDeviceMIDIEvent(m_synthUnit, MIDI_STATUS_PROGRAMCHANGE+i, 0,0,0);
                    registerStatus( "MusicDeviceMIDIEvent", result );
                }

                result = AUGraphStart (m_graph);
                registerStatus( "AUGraphStart", result);
                if (result != noErr) {
                    return;
                }
            }
            m_connection = QString(QLatin1String(PRETTY_NAME));
        }

        void stop ()
        {
            OSStatus result;
            //qDebug() << Q_FUNC_INFO;
            if (m_graph != nullptr) {
                result = AUGraphStop(m_graph);
                if (result != noErr)
                    qWarning() << "AUGraphStop() err:" << result;
                result = AUGraphClose(m_graph);
                if (result != noErr)
                    qWarning() << "AUGraphClose() err:" << result;
                result = DisposeAUGraph(m_graph);
                if (result != noErr)
                    qWarning() << "DisposeAUGraph() err:" << result;
                m_graph = nullptr;
            }
            m_connection.clear();
        }

        void initialize ( QSettings *settings )
        {
            //qDebug() << Q_FUNC_INFO;
            settings->beginGroup(PRETTY_NAME);
            m_default_dls = settings->value("default_dls", true).toBool();
            m_reverb_dls = settings->value("reverb_dls", false).toBool();
            m_soundfont_dls = settings->value("soundfont_dls").toString();
            //qDebug() << "default_dls:" << m_default_dls;
            //qDebug() << "reverb_dls:" << m_reverb_dls;
            //qDebug() << "soundfont_dls" << m_soundfont_dls;
            settings->endGroup();
        }

        void registerStatus(const QString& context, const long status)
        {
            if (status != noErr) {
                qWarning() << context << "err:" << status;
            } //else {
            //    qDebug() << context;
            //}
        }

        void sendStatusEvent(uint status, uint data1, uint data2)
        {
            MusicDeviceMIDIEvent ( m_synthUnit, status, data1, data2, 0 );
        }

        void sendSysexEvent(Byte *msg, uint msglen)
        {
            MusicDeviceSysEx ( m_synthUnit, msg, msglen );
        }
    };


    MacSynthOutput::MacSynthOutput(QObject *parent):
        MIDIOutput(parent), d(new MacSynthOutputPrivate)
    {
    }

    MacSynthOutput::~MacSynthOutput()
    {
        delete d;
    }

    void MacSynthOutput::initialize(QSettings* settings)
    {
        d->initialize(settings);
    }

    QString MacSynthOutput::backendName()
    {
        return QLatin1String(PRETTY_NAME);
    }

    QString MacSynthOutput::publicName()
    {
        return QLatin1String(PRETTY_NAME);
    }

    void MacSynthOutput::setPublicName(QString name)
    {
        Q_UNUSED(name)
    }

    QStringList MacSynthOutput::connections(bool advanced)
    {
        Q_UNUSED(advanced)
        return QStringList(QLatin1String(PRETTY_NAME));
    }

    void MacSynthOutput::setExcludedConnections(QStringList conns)
    {
        Q_UNUSED(conns)
    }

    void MacSynthOutput::open(QString name)
    {
        Q_UNUSED(name)
        //qDebug() << Q_FUNC_INFO;
        d->start();
    }

    void MacSynthOutput::close()
    {
        //qDebug() << Q_FUNC_INFO;
        d->stop();
    }

    QString MacSynthOutput::currentConnection()
    {
        return d->currentConnection();
    }

    void MacSynthOutput::sendNoteOn(int chan, int note, int vel)
    {
        uint status, data1, data2;
        status = MIDI_STATUS_NOTEON | (chan & 0x0f);
        data1 = static_cast<unsigned>(note);
        data2 = static_cast<unsigned>(vel);
        d->sendStatusEvent(status, data1, data2);
    }

    void MacSynthOutput::sendNoteOff(int chan, int note, int vel)
    {
        uint status, data1, data2;
        status = MIDI_STATUS_NOTEOFF | (chan & 0x0f);
        data1 = static_cast<unsigned>(note);
        data2 = static_cast<unsigned>(vel);
        d->sendStatusEvent(status, data1, data2);
    }

    void MacSynthOutput::sendController(int chan, int control, int value)
    {
        uint status, data1, data2;
        status = MIDI_STATUS_CONTROLCHANGE | (chan & 0x0f);
        data1 = static_cast<unsigned>(control);
        data2 = static_cast<unsigned>(value);
        d->sendStatusEvent(status, data1, data2);
    }

    void MacSynthOutput::sendKeyPressure(int chan, int note, int value)
    {
        uint status, data1, data2;
        status = MIDI_STATUS_KEYPRESURE | (chan & 0x0f);
        data1 = static_cast<unsigned>(note);
        data2 = static_cast<unsigned>(value);
        d->sendStatusEvent(status, data1, data2);
    }

    void MacSynthOutput::sendProgram(int chan, int program)
    {
        uint status, data1, data2;
        status = MIDI_STATUS_PROGRAMCHANGE | (chan & 0x0f);
        data1 = static_cast<unsigned>(program);
        data2 = 0;
        d->sendStatusEvent(status, data1, data2);
    }

    void MacSynthOutput::sendChannelPressure(int chan, int value)
    {
        uint status, data1, data2;
        status = MIDI_STATUS_CHANNELPRESSURE | (chan & 0x0f);
        data1 = static_cast<unsigned>(value);
        data2 = 0;
        d->sendStatusEvent(status, data1, data2);
    }

    void MacSynthOutput::sendPitchBend(int chan, int value)
    {
        uint status, data1, data2;
        quint16 val = static_cast<uint16>(value + 8192); // value between -8192 and +8191
        status = MIDI_STATUS_PITCHBEND | (chan & 0x0f);
        data1 = MIDI_LSB(val); // LSB
        data2 = MIDI_MSB(val); // MSB
        d->sendStatusEvent(status, data1, data2);
    }

    void MacSynthOutput::sendSysex(const QByteArray& data)
    {
        d->sendSysexEvent(reinterpret_cast<unsigned char *>(const_cast<char*>(data.data())), static_cast<unsigned>(data.length()));
    }

    void MacSynthOutput::sendSystemMsg(const int status)
    {
        d->sendStatusEvent(static_cast<unsigned>(status), 0, 0);
    }

}}
