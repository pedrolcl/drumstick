/*
    Drumstick RT Windows Backend
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

#include <QStringList>
#include <QByteArray>
#include <QVarLengthArray>
#include <qmath.h>
#include <windows.h>
#include <mmsystem.h>
#include "winmidioutput.h"

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
#define hex Qt::hex
#endif

namespace drumstick {
namespace rt {

    union WinMIDIPacket {
        WinMIDIPacket() : dwPacket(0) {}
        DWORD dwPacket;
        quint8 data[sizeof(DWORD)];
    };

    static QLatin1String DEFAULT_PUBLIC_NAME = QLatin1String("MIDI Out");

    void CALLBACK midiCallback( HMIDIOUT hmo,
                                UINT wMsg,
                                DWORD_PTR dwInstance,
                                DWORD_PTR dwParam1,
                                DWORD_PTR dwParam2);

    class WinMIDIOutput::WinMIDIOutputPrivate {
    public:
        HMIDIOUT m_handle;
        bool m_clientFilter;
        QString m_publicName;
        MIDIConnection m_currentOutput;
        QList<MIDIConnection> m_outputDevices;
        MIDIHDR m_midiSysexHdr;
        QByteArray m_sysexBuffer;
        QStringList m_excludedNames;
        bool m_status;
        QStringList m_diagnostics;

        WinMIDIOutputPrivate():
            m_handle(nullptr),
            m_clientFilter(true),
            m_publicName(DEFAULT_PUBLIC_NAME)
        {
            reloadDeviceList(true);
        }

        void reloadDeviceList(bool advanced)
        {
            MMRESULT res;
            MIDIOUTCAPS deviceCaps;
            QString devName;
            unsigned int dev, max = midiOutGetNumDevs();
            m_outputDevices.clear();
            m_clientFilter = !advanced;

            for ( dev = 0; dev < max; ++dev) {
                bool excluded = false;
                res = midiOutGetDevCaps( dev, &deviceCaps, sizeof(MIDIOUTCAPS));
                if (res != MMSYSERR_NOERROR)
                    break;
                if (m_clientFilter && (deviceCaps.wTechnology == MOD_MAPPER || dev == MIDI_MAPPER))
                    continue;
    #if defined(UNICODE)
                devName = QString::fromWCharArray(deviceCaps.szPname);
    #else
                devName = QString::fromLocal8Bit(deviceCaps.szPname);
    #endif
                foreach (const QString& n, m_excludedNames) {
                    if (devName.startsWith(n)) {
                        excluded = true;
                        break;
                    }
                }

                if (!excluded) {
                    m_outputDevices << MIDIConnection(devName, dev);
                }
            }
            if (!m_clientFilter) {
                dev = MIDI_MAPPER;
                res = midiOutGetDevCaps( dev, &deviceCaps, sizeof(MIDIOUTCAPS));
                if (res == MMSYSERR_NOERROR) {
    #if defined(UNICODE)
                    devName = QString::fromWCharArray(deviceCaps.szPname);
    #else
                    devName = QString::fromLocal8Bit(deviceCaps.szPname);
    #endif
                    m_outputDevices << MIDIConnection(devName, dev);
                }
            }
        }

        void setPublicName(QString name)
        {
            if (m_publicName != name) {
                m_publicName = name;
            }
        }

        void open(const MIDIConnection& conn)
        {
            MMRESULT res;
            m_status = false;
            m_diagnostics.clear();
            if (m_handle != nullptr)
                close();
            reloadDeviceList(!m_clientFilter);
            if (!conn.first.isEmpty()) {
                res = midiOutOpen( &m_handle, conn.second.toInt(), (DWORD_PTR) midiCallback, (DWORD_PTR) this, CALLBACK_FUNCTION);
                if (res == MMSYSERR_NOERROR) {
                    m_currentOutput = conn;
                    m_status = true;
                } else {
                    logError("midiStreamOpen()", res);
                }
            }
        }

        void close()
        {
            MMRESULT res;
            m_status = false;
            m_diagnostics.clear();
            if (m_handle != nullptr) {
                res = midiOutReset( m_handle );
                if (res != MMSYSERR_NOERROR)
                    logError("midiOutReset()", res);
                res = midiOutClose( m_handle );
                if (res == MMSYSERR_NOERROR)
                    m_currentOutput = MIDIConnection();
                else
                    logError("midiStreamClose()", res);
                m_handle = nullptr;
            }
        }

        void doneHeader( LPMIDIHDR lpMidiHdr )
        {
            MMRESULT res;
            res = midiOutUnprepareHeader( m_handle, lpMidiHdr, sizeof(MIDIHDR) );
            if (res != MMSYSERR_NOERROR)
                logError("midiOutUnprepareHeader()", res);
            if ((lpMidiHdr->dwFlags & MHDR_ISSTRM) == 0)
                return; // sysex header?
        }

        void sendShortMessage(WinMIDIPacket &packet)
        {
            MMRESULT res;
            res = midiOutShortMsg( m_handle, packet.dwPacket );
            if ( res != MMSYSERR_NOERROR )
                logError("midiOutShortMsg()", res);
        }

        void sendSysexEvent(const QByteArray& data)
        {
            MMRESULT res;
            m_sysexBuffer = data;
            m_midiSysexHdr.lpData = (LPSTR) m_sysexBuffer.data();
            m_midiSysexHdr.dwBufferLength = m_sysexBuffer.size();
            m_midiSysexHdr.dwBytesRecorded = m_sysexBuffer.size();
            m_midiSysexHdr.dwFlags = 0;
            m_midiSysexHdr.dwUser = 0;
            res = midiOutPrepareHeader( m_handle, &m_midiSysexHdr, sizeof(MIDIHDR) );
            if (res != MMSYSERR_NOERROR)
                logError("midiOutPrepareHeader()", res);
            else {
                res = midiOutLongMsg( m_handle, &m_midiSysexHdr, sizeof(MIDIHDR) );
                if (res != MMSYSERR_NOERROR)
                    logError("midiOutLongMsg()", res);
            }
        }

        QString mmErrorString(MMRESULT err)
        {
            QString errstr;
    #ifdef UNICODE
            WCHAR buffer[1024];
            midiOutGetErrorText(err, &buffer[0], sizeof(buffer));
            errstr = QString::fromWCharArray(buffer);
    #else
            char buffer[1024];
            midiOutGetErrorText(err, &buffer[0], sizeof(buffer));
            errstr = QString::fromLocal8Bit(buffer);
    #endif
            return errstr;
        }

        void logError(const QString context, const MMRESULT code)
        {
            m_diagnostics << QString("%1 error: %2 (%3)").arg(context).arg(code).arg(mmErrorString(code));
        }
    };


    void CALLBACK midiCallback( HMIDIOUT hmo,
                                UINT wMsg,
                                DWORD_PTR dwInstance,
                                DWORD_PTR dwParam1,
                                DWORD_PTR dwParam2)
    {
        Q_UNUSED(hmo)
        Q_UNUSED(dwParam2)
        WinMIDIOutput::WinMIDIOutputPrivate* obj = (WinMIDIOutput::WinMIDIOutputPrivate*) dwInstance;
        switch( wMsg ) {
        case MOM_DONE:
            obj->doneHeader( (LPMIDIHDR) dwParam1 );
            break;
        case MOM_OPEN:
            obj->m_diagnostics << "Open output";
            break;
        case MOM_CLOSE:
            obj->m_diagnostics << "Close output";
            break;
        default:
            obj->m_diagnostics << QString("unknown output message: %1").arg(wMsg);
            break;
        }
    }

    WinMIDIOutput::WinMIDIOutput(QObject *parent) :
        MIDIOutput(parent), d(new WinMIDIOutputPrivate)
    { }

    WinMIDIOutput::~WinMIDIOutput()
    {
        delete d;
    }

    void WinMIDIOutput::initialize(QSettings *settings)
    {
        Q_UNUSED(settings)
    }

    QString WinMIDIOutput::backendName()
    {
        return "Windows MM";
    }

    QString WinMIDIOutput::publicName()
    {
        return d->m_publicName;
    }

    void WinMIDIOutput::setPublicName(QString name)
    {
        d->setPublicName(name);
    }

    QList<MIDIConnection> WinMIDIOutput::connections(bool advanced)
    {
        d->reloadDeviceList(advanced);
        return d->m_outputDevices;
    }

    void WinMIDIOutput::setExcludedConnections(QStringList conns)
    {
        d->m_excludedNames = conns;
    }

    void WinMIDIOutput::open(const MIDIConnection& name)
    {
        d->open(name);
    }

    void WinMIDIOutput::close()
    {
        d->close();
    }

    MIDIConnection WinMIDIOutput::currentConnection()
    {
        return d->m_currentOutput;
    }

    void WinMIDIOutput::sendNoteOn(int chan, int note, int vel)
    {
        WinMIDIPacket packet;
        packet.data[0] = MIDI_STATUS_NOTEON | (chan & MIDI_CHANNEL_MASK);
        packet.data[1] = note;
        packet.data[2] = vel;
        d->sendShortMessage(packet);
    }

    void WinMIDIOutput::sendNoteOff(int chan, int note, int vel)
    {
        WinMIDIPacket packet;
        packet.data[0] = MIDI_STATUS_NOTEOFF | (chan & MIDI_CHANNEL_MASK);
        packet.data[1] = note;
        packet.data[2] = vel;
        d->sendShortMessage(packet);
    }

    void WinMIDIOutput::sendController(int chan, int control, int value)
    {
        WinMIDIPacket packet;
        packet.data[0] = MIDI_STATUS_CONTROLCHANGE | (chan & MIDI_CHANNEL_MASK);
        packet.data[1] = control;
        packet.data[2] = value;
        d->sendShortMessage(packet);
    }

    void WinMIDIOutput::sendKeyPressure(int chan, int note, int value)
    {
        WinMIDIPacket packet;
        packet.data[0] = MIDI_STATUS_KEYPRESURE | (chan & MIDI_CHANNEL_MASK);
        packet.data[1] = note;
        packet.data[2] = value;
        d->sendShortMessage(packet);
    }

    void WinMIDIOutput::sendProgram(int chan, int program)
    {
        WinMIDIPacket packet;
        packet.data[0] = MIDI_STATUS_PROGRAMCHANGE | (chan & MIDI_CHANNEL_MASK);
        packet.data[1] = program;
        d->sendShortMessage(packet);
    }

    void WinMIDIOutput::sendChannelPressure(int chan, int value)
    {
        WinMIDIPacket packet;
        packet.data[0] = MIDI_STATUS_CHANNELPRESSURE | (chan & MIDI_CHANNEL_MASK);
        packet.data[1] = value;
        d->sendShortMessage(packet);
    }

    void WinMIDIOutput::sendPitchBend(int chan, int v)
    {
        WinMIDIPacket packet;
        // -8192 <= v <= 8191; 0 <= value <= 16384
        int value = 8192 + v;
        packet.data[0] = MIDI_STATUS_PITCHBEND | (chan & MIDI_CHANNEL_MASK);
        packet.data[1] = MIDI_LSB(value);
        packet.data[2] = MIDI_MSB(value);
        d->sendShortMessage(packet);
    }

    void WinMIDIOutput::sendSystemMsg(const int status)
    {
        WinMIDIPacket packet;
        packet.data[0] = status;
        d->sendShortMessage(packet);
    }

    void WinMIDIOutput::sendSysex(const QByteArray &data)
    {
        d->sendSysexEvent(data);
    }

    QStringList WinMIDIOutput::getDiagnostics()
    {
        return d->m_diagnostics;
    }

    bool WinMIDIOutput::getStatus()
    {
        return d->m_status;
    }
}}
