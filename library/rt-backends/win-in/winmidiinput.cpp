/*
    Drumstick RT Windows Backend
    Copyright (C) 2009-2023 Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include <QString>
#include <QMap>
#include <QDebug>
#include <windows.h>
#include <mmsystem.h>

#include "winmidiinput.h"

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
#define right Qt::right
#define left Qt::left
#define hex Qt::hex
#define dec Qt::dec
#define endl Qt::endl
#endif

namespace drumstick {
namespace rt {

    static QLatin1String DEFAULT_PUBLIC_NAME = QLatin1String("MIDI In");

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
        MIDIConnection m_currentInput;
        QStringList m_excludedNames;
        QList<MIDIConnection> m_inputDevices;
        bool m_status;
        QStringList m_diagnostics;

        #define SYSEX_BUF_SIZE 32768
        MIDIHDR inMidiHeader;
        unsigned char inSysexBuf[SYSEX_BUF_SIZE];

        explicit WinMIDIInputPrivate(WinMIDIInput *inp):
            m_inp(inp),
            m_out(nullptr),
            m_thruEnabled(false),
            m_clientFilter(true),
            m_handle(nullptr),
            m_publicName(DEFAULT_PUBLIC_NAME)
        {
            reloadDeviceList(true);
        }

        void open(const MIDIConnection &conn) {
            MMRESULT res;
            m_status = false;
            m_diagnostics.clear();
            if (conn != m_currentInput) {
                if (m_handle != nullptr)
                    close();
                reloadDeviceList(!m_clientFilter);
                if (!conn.first.isEmpty()) {
                    res = midiInOpen(&m_handle, conn.second.toInt(), (DWORD_PTR) midiCallback, (DWORD_PTR) this, CALLBACK_FUNCTION | MIDI_IO_STATUS );
                    if (res != MMSYSERR_NOERROR) {
                        logError("midiInOpen()",res);
                    } else {

                    /* Double buffering configuration, to manage Sysexes */
                    /* Store pointer to our input buffer for System Exclusive messages in MIDIHDR */
                    inMidiHeader.lpData = (LPSTR)&inSysexBuf[0];

                    /* Store its size in the MIDIHDR */
                    inMidiHeader.dwBufferLength = SYSEX_BUF_SIZE;

                    /* Flags must be set to 0 */
                    inMidiHeader.dwFlags = 0;

                    res = midiInPrepareHeader(m_handle, &inMidiHeader, sizeof(MIDIHDR));
                    if (res != MMSYSERR_NOERROR){
                        qDebug() << "Error preparing MIDI input header. result=" << res;
                    }

                    res = midiInAddBuffer(m_handle, &inMidiHeader, sizeof(MIDIHDR));
                    if (res != MMSYSERR_NOERROR){
                        qDebug() << "Error adding MIDI input buffer. result=" << res;
                    }
                    /*** ***/


                        res = midiInStart(m_handle);
                        if (res != MMSYSERR_NOERROR) {
                            logError("midiInStart()",res);
                        }
                    }
                    m_currentInput = conn;
                    m_status = (res == MMSYSERR_NOERROR);
                }
            }
        }

        void close() {
            MMRESULT res;
            m_status = false;
            m_diagnostics.clear();
            if (m_handle != nullptr) {
                res = midiInStop(m_handle);
                if (res != MMSYSERR_NOERROR)
                    logError("midiInStop()", res);
                res = midiInReset( m_handle );
                if (res != MMSYSERR_NOERROR)
                    logError("midiInReset()", res);
                res = midiInClose( m_handle );
                if (res != MMSYSERR_NOERROR)
                    logError("midiInClose()", res);
                m_handle = nullptr;
            }
            m_currentInput = MIDIConnection();
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
                foreach (const QString& n , m_excludedNames) {
                    if (devName.startsWith(n)) {
                        excluded = true;
                        break;
                    }
                }
                if (!excluded) {
                    m_inputDevices << MIDIConnection(devName, dev);
                }
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
                if(m_out != nullptr && m_thruEnabled)
                    m_out->sendNoteOff(channel, data1, data2);
                emit m_inp->midiNoteOff(channel, data1, data2);
                break;
            case MIDI_STATUS_NOTEON:
                if(m_out != nullptr && m_thruEnabled)
                    m_out->sendNoteOn(channel, data1, data2);
                emit m_inp->midiNoteOn(channel, data1, data2);
                break;
            case MIDI_STATUS_KEYPRESURE:
                if(m_out != nullptr && m_thruEnabled)
                    m_out->sendKeyPressure(channel, data1, data2);
                emit m_inp->midiKeyPressure(channel, data1, data2);
                break;
            case MIDI_STATUS_CONTROLCHANGE:
                if(m_out != nullptr && m_thruEnabled)
                    m_out->sendController(channel, data1, data2);
                emit m_inp->midiController(channel, data1, data2);
                break;
            case MIDI_STATUS_PROGRAMCHANGE:
                if(m_out != nullptr && m_thruEnabled)
                    m_out->sendProgram(channel, data1);
                emit m_inp->midiProgram(channel, data1);
                break;
            case MIDI_STATUS_CHANNELPRESSURE:
                if(m_out != nullptr && m_thruEnabled)
                    m_out->sendChannelPressure(channel, data1);
                emit m_inp->midiChannelPressure(channel, data1);
                break;
            case MIDI_STATUS_PITCHBEND: {
                    int value = (data1 + data2 * 0x80) - 8192;
                    if(m_out != nullptr && m_thruEnabled)
                        m_out->sendPitchBend(channel, value);
                    emit m_inp->midiPitchBend(channel, value);
                }
                break;
            default:
                m_diagnostics << QString("MIDI in status? %1").arg(status);
            }
        }

        void emitSysex(QByteArray data)
        {
            if(m_out != nullptr && m_thruEnabled)
                m_out->sendSysex(data);
            emit m_inp->midiSysex(data);
        }

        QString mmErrorString(MMRESULT err)
        {
            QString errstr;
    #ifdef UNICODE
            WCHAR buffer[1024];
            midiInGetErrorText(err, &buffer[0], sizeof(buffer));
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

    void CALLBACK midiCallback( HMIDIIN hMidiIn,
                                UINT wMsg,
                                DWORD_PTR dwInstance,
                                DWORD_PTR dwParam1,
                                DWORD_PTR dwParam2 )
    {
        Q_UNUSED(hMidiIn)
        Q_UNUSED(dwParam2)
        WinMIDIInput::WinMIDIInputPrivate* object = (WinMIDIInput::WinMIDIInputPrivate*) dwInstance;
        switch( wMsg ) {
        case MIM_OPEN:
            object->m_diagnostics << "Open input";
            break;
        case MIM_CLOSE:
            object->m_diagnostics << "Close input";
            break;
        case MIM_ERROR:
        case MIM_LONGERROR:
            object->m_diagnostics << "Errors input";
            break;
        case MIM_LONGDATA:
	{
            object->m_diagnostics << "Sysex data input";
            // Extract MIDI SysEx message information
            MIDIHDR* pMidiHdr = reinterpret_cast<MIDIHDR*>(dwParam1);

            // Check for SysEx messages
            if (pMidiHdr->dwBytesRecorded > 0 && (unsigned char)pMidiHdr->lpData[0] == 0xF0) // Start of SysEx
            {
                QByteArray sysExByteArray(reinterpret_cast<const char*>(pMidiHdr->lpData), static_cast<int>(pMidiHdr->dwBytesRecorded));
                object->emitSysex(sysExByteArray);
            }

            /* Queue the MIDIHDR for more input */
            midiInAddBuffer(hMidiIn, pMidiHdr, sizeof(MIDIHDR));
            break;
        }
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
            object->m_diagnostics << QString("unknown input message: %1").arg(wMsg);
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
        return QLatin1String("Windows MM");
    }

    QString WinMIDIInput::publicName()
    {
        return d->m_publicName;
    }

    void WinMIDIInput::setPublicName(QString name)
    {
        d->setPublicName(name);
    }

    QList<MIDIConnection> WinMIDIInput::connections(bool advanced)
    {
        d->reloadDeviceList(advanced);
        return d->m_inputDevices;
    }

    void WinMIDIInput::setExcludedConnections(QStringList conns)
    {
        d->m_excludedNames = conns;
    }

    void WinMIDIInput::open(const MIDIConnection& name)
    {
        d->open(name);
    }

    void WinMIDIInput::close()
    {
        d->close();
    }

    MIDIConnection WinMIDIInput::currentConnection()
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
        return d->m_thruEnabled && d->m_out != nullptr;
    }

    QStringList WinMIDIInput::getDiagnostics()
    {
        return d->m_diagnostics;
    }

    bool WinMIDIInput::getStatus()
    {
        return d->m_status;
    }
}} // namespace drumstick::rt
