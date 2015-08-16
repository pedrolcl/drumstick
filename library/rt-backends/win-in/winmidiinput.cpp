/*
    Drumstick RT Windows Backend
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
#include <QMap>
#include <QDebug>
#include <windows.h>
#include <mmsystem.h>

#include "winmidiinput.h"

namespace drumstick {
namespace rt {

    static QLatin1Literal DEFAULT_PUBLIC_NAME = QLatin1Literal("MIDI In");

    void CALLBACK midiCallback( HMIDIIN hMidiIn,
                                UINT wMsg,
                                DWORD_PTR dwInstance,
                                DWORD_PTR dwParam1,
                                DWORD_PTR dwParam2 );


    class WinMIDIInput::WinMIDIInputPrivate {
    public:
        WinMIDIInput *m_inp;
        MIDIOutput *m_out;
        bool m_thruEnabled;
        bool m_clientFilter;
        HMIDIIN m_handle;
        QString m_publicName;
        QString m_currentInput;
        QStringList m_excludedNames;
        QMap<int,QString> m_inputDevices;

        WinMIDIInputPrivate(WinMIDIInput *inp):
            m_inp(inp),
            m_out(0),
            m_thruEnabled(false),
            m_clientFilter(true),
            m_handle(0),
            m_publicName(DEFAULT_PUBLIC_NAME)
        {
            reloadDeviceList(true);
        }

        int deviceIndex( const QString& newDevice )
        {
            int index = -1;
            QMap<int,QString>::ConstIterator it;
            for( it = m_inputDevices.constBegin();
                 it != m_inputDevices.constEnd(); ++it ) {
                if (it.value() == newDevice) {
                    index = it.key();
                    break;
                }
            }
            return index;
        }

        void open(QString name) {
            MMRESULT res;
            if (name != m_currentInput) {
                if (m_handle != 0)
                    close();
                reloadDeviceList(!m_clientFilter);
                int dev = deviceIndex(name);
                if (dev > -1) {
                    res = midiInOpen(&m_handle, dev, (DWORD_PTR) midiCallback, (DWORD_PTR) this, CALLBACK_FUNCTION | MIDI_IO_STATUS );
                    if (res != MMSYSERR_NOERROR)
                        qDebug() << "midiInOpen() err:" << mmErrorString(res);
                    res = midiInStart(m_handle);
                    if (res != MMSYSERR_NOERROR)
                        qDebug() << "midiInStart() err:" << mmErrorString(res);
                    m_currentInput = name;
                }
            }
        }

        void close() {
            MMRESULT res;
            if (m_handle != 0) {
                res = midiInStop(m_handle);
                if (res != MMSYSERR_NOERROR)
                    qDebug() << "midiInStop() err:" << mmErrorString(res);
                res = midiInReset( m_handle );
                if (res != MMSYSERR_NOERROR)
                    qDebug() << "midiInReset() err:" << mmErrorString(res);
                res = midiInClose( m_handle );
                if (res != MMSYSERR_NOERROR)
                    qDebug() << "midiInClose() err:" << mmErrorString(res);
                m_handle = 0;
            }
            m_currentInput.clear();
        }

        void reloadDeviceList(bool advanced)
        {
            MMRESULT res;
            MIDIINCAPS deviceCaps;
            QString devName;
            unsigned int dev, max = midiInGetNumDevs();
            m_inputDevices.clear();
            m_clientFilter = !advanced;

            for ( dev = 0; dev < max; ++dev) {
                bool excluded = false;
                res = midiInGetDevCaps( dev, &deviceCaps, sizeof(MIDIINCAPS));
                if (res != MMSYSERR_NOERROR)
                    break;
#if defined(UNICODE)
                devName = QString::fromWCharArray(deviceCaps.szPname);
#else
                devName = QString::fromLocal8Bit(deviceCaps.szPname);
#endif
                foreach(const QString& n, m_excludedNames) {
                    if (devName.startsWith(n)) {
                        excluded = true;
                        break;
                    }
                }
                if (!excluded)
                    m_inputDevices[dev] = devName;
            }
        }

        void setPublicName(QString name)
        {
            if (m_publicName != name) {
                m_publicName = name;
            }
        }

        void emitSignals(int status, int channel, int data1, int data2)
        {
            switch (status) {
            case MIDI_STATUS_NOTEOFF:
                if(m_out != 0 && m_thruEnabled)
                    m_out->sendNoteOff(channel, data1, data2);
                emit m_inp->midiNoteOff(channel, data1, data2);
                break;
            case MIDI_STATUS_NOTEON:
                if(m_out != 0 && m_thruEnabled)
                    m_out->sendNoteOn(channel, data1, data2);
                emit m_inp->midiNoteOn(channel, data1, data2);
                break;
            case MIDI_STATUS_KEYPRESURE:
                if(m_out != 0 && m_thruEnabled)
                    m_out->sendKeyPressure(channel, data1, data2);
                emit m_inp->midiKeyPressure(channel, data1, data2);
                break;
            case MIDI_STATUS_CONTROLCHANGE:
                if(m_out != 0 && m_thruEnabled)
                    m_out->sendController(channel, data1, data2);
                emit m_inp->midiController(channel, data1, data2);
                break;
            case MIDI_STATUS_PROGRAMCHANGE:
                if(m_out != 0 && m_thruEnabled)
                    m_out->sendProgram(channel, data1);
                emit m_inp->midiProgram(channel, data1);
                break;
            case MIDI_STATUS_CHANNELPRESSURE:
                if(m_out != 0 && m_thruEnabled)
                    m_out->sendChannelPressure(channel, data1);
                emit m_inp->midiChannelPressure(channel, data1);
                break;
            case MIDI_STATUS_PITCHBEND: {
                    int value = (data1 + data2 * 0x80) - 8192;
                    if(m_out != 0 && m_thruEnabled)
                        m_out->sendPitchBend(channel, value);
                    emit m_inp->midiPitchBend(channel, value);
                }
                break;
            default:
                qDebug() << "MIDI in status?" << status;
            }
        }

        void emitSysex(QByteArray data)
        {
            if(m_out != 0 && m_thruEnabled)
                m_out->sendSysex(data);
            emit m_inp->midiSysex(data);
        }

        QString mmErrorString(MMRESULT err)
        {
            QString errstr;
    #ifdef UNICODE
            WCHAR buffer[1024];
            midiInGetErrorText(err, &buffer[0], sizeof(buffer));
            errstr = QString::fromUtf16((const ushort*)buffer);
    #else
            char buffer[1024];
            midiOutGetErrorText(err, &buffer[0], sizeof(buffer));
            errstr = QString::fromLocal8Bit(buffer);
    #endif
            return errstr;
        }

    };

    void CALLBACK midiCallback( HMIDIIN hMidiIn,
                                UINT wMsg,
                                DWORD_PTR dwInstance,
                                DWORD_PTR dwParam1,
                                DWORD_PTR dwParam2 )
    {
        //Q_UNUSED(hMidiIn)
        Q_UNUSED(dwParam2)
        WinMIDIInput::WinMIDIInputPrivate* object = (WinMIDIInput::WinMIDIInputPrivate*) dwInstance;
        switch( wMsg ) {
        case MIM_OPEN:
            qDebug() << "Open input" << hMidiIn;
            break;
        case MIM_CLOSE:
            qDebug() << "Close input" << hMidiIn;
            break;
        case MIM_ERROR:
        case MIM_LONGERROR:
            qDebug() << "Errors input";
            break;
        case MIM_LONGDATA:
            qDebug() << "Sysex data input";
            break;
        case MIM_DATA:
        case MIM_MOREDATA: {
                int status = dwParam1 & 0xf0;
                int channel = dwParam1 & 0x0f;
                int data1 = (dwParam1 & 0x7f00) >> 8;
                int data2 = (dwParam1 & 0x7f0000) >> 16;
                object->emitSignals(status, channel, data1, data2);
            }
            break;
        default:
            qDebug() << "unknown input message:" << hex << wMsg;
            break;
        }
    }

    WinMIDIInput::WinMIDIInput(QObject *parent) :
        MIDIInput(parent), d(new WinMIDIInputPrivate(this))
    { }

    WinMIDIInput::~WinMIDIInput()
    {
        delete d;
    }

    void WinMIDIInput::initialize(QSettings *settings)
    {
        Q_UNUSED(settings)
    }

    QString WinMIDIInput::backendName()
    {
        return QLatin1Literal("Windows MM");
    }

    QString WinMIDIInput::publicName()
    {
        return d->m_publicName;
    }

    void WinMIDIInput::setPublicName(QString name)
    {
        d->setPublicName(name);
    }

    QStringList WinMIDIInput::connections(bool advanced)
    {
        d->reloadDeviceList(advanced);
        return d->m_inputDevices.values();
    }

    void WinMIDIInput::setExcludedConnections(QStringList conns)
    {
        d->m_excludedNames = conns;
    }

    void WinMIDIInput::open(QString name)
    {
        d->open(name);
    }

    void WinMIDIInput::close()
    {
        d->close();
    }

    QString WinMIDIInput::currentConnection()
    {
        return d->m_currentInput;
    }

    void WinMIDIInput::setMIDIThruDevice(MIDIOutput *device)
    {
        d->m_out = device;
    }

    void WinMIDIInput::enableMIDIThru(bool enable)
    {
        d->m_thruEnabled = enable;
    }

    bool WinMIDIInput::isEnabledMIDIThru()
    {
        return d->m_thruEnabled && d->m_out != 0;
    }

}} // namespace drumstick::rt
