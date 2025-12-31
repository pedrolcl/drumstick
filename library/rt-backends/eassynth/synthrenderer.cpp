/*
    Sonivox EAS Synthesizer for Qt applications
    Copyright (C) 2016-2025, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <QCoreApplication>
#include <QObject>
#include <QReadLocker>
#include <QString>
#include <QSysInfo>
#include <QTextStream>
#include <QVersionNumber>
#include <QWriteLocker>

#include <eas_chorus.h>
#include <eas_reverb.h>
#include <pulse/simple.h>
#include "synthrenderer.h"
#include "filewrapper.h"

namespace drumstick {
namespace rt {

const QString SynthRenderer::QSTR_PREFERENCES = QStringLiteral("SonivoxEAS");
const QString SynthRenderer::QSTR_BUFFERTIME = QStringLiteral("BufferTime");
const QString SynthRenderer::QSTR_REVERBTYPE = QStringLiteral("ReverbType");
const QString SynthRenderer::QSTR_REVERBAMT = QStringLiteral("ReverbAmt");
const QString SynthRenderer::QSTR_CHORUSTYPE = QStringLiteral("ChorusType");
const QString SynthRenderer::QSTR_CHORUSAMT = QStringLiteral("ChorusAmt");
const QString SynthRenderer::QSTR_SONIVOXEAS = QStringLiteral("SonivoxEAS");
const QString SynthRenderer::QSTR_SOUNDFONT = QStringLiteral("InstrumentsDefinition");

const int SynthRenderer::DEF_BUFFERTIME = 60;
const int SynthRenderer::DEF_REVERBTYPE = EAS_PARAM_REVERB_HALL;
const int SynthRenderer::DEF_REVERBAMT = 25800;
const int SynthRenderer::DEF_CHORUSTYPE = -1;
const int SynthRenderer::DEF_CHORUSAMT = 0;

SynthRenderer::SynthRenderer(QObject *parent) : QObject(parent),
    m_Stopped(true),
    m_rendering(nullptr),
    m_easData(nullptr),
    m_streamHandle(nullptr),
    m_bufferTime(60)
{ }

void
SynthRenderer::initEAS()
{
    /* SONiVOX EAS initialization */
    EAS_RESULT eas_res;
    EAS_DATA_HANDLE dataHandle;
    EAS_HANDLE handle;
    m_status = false;
    m_diagnostics.clear();

    const S_EAS_LIB_CONFIG *easConfig = EAS_Config();
    if (easConfig == nullptr) {
        m_diagnostics << "EAS_Config returned null";
        return;
    }
    m_sampleRate = easConfig->sampleRate;
    m_bufferSize = easConfig->mixBufferSize;
    m_channels = easConfig->numChannels;
    m_libVersion = easConfig->libVersion;

    eas_res = EAS_Init(&dataHandle);
    if (eas_res != EAS_SUCCESS) {
      m_diagnostics << QString("EAS_Init error: %1").arg( eas_res );
      return;
    }
    m_easData = dataHandle;

    if (!m_soundfont.isEmpty()) {
        FileWrapper dlsFile(m_soundfont);
        if (dlsFile.ok()) {
            eas_res = EAS_LoadDLSCollection(dataHandle, nullptr, dlsFile.getLocator());
            if (eas_res != EAS_SUCCESS) {
                m_diagnostics << QString("EAS_LoadDLSCollection(%1) error: %2").arg(m_soundfont).arg(eas_res);
            }
        } else {
            m_diagnostics << QString("Failed to open %1").arg(m_soundfont);
        }
    }

    eas_res = EAS_OpenMIDIStream(dataHandle, &handle, nullptr);
    if (eas_res != EAS_SUCCESS) {
      m_diagnostics << QString("EAS_OpenMIDIStream error: %1").arg( eas_res );
      EAS_Shutdown(dataHandle);
      return;
    }
    m_streamHandle = handle;
    Q_ASSERT(m_streamHandle != nullptr);

    m_status = true;
    //qDebug() << Q_FUNC_INFO << "EAS bufferSize=" << m_bufferSize << " sampleRate=" << m_sampleRate << " channels=" << m_channels;
}

void
SynthRenderer::initPulse()
{
    pa_sample_spec samplespec;
    pa_buffer_attr bufattr;
    int period_bytes;
    char *server = nullptr;
    char *device = nullptr;
    int err;

    samplespec.format = PA_SAMPLE_S16LE;
    samplespec.channels = m_channels;
    samplespec.rate = m_sampleRate;

    //period_bytes = m_bufferSize * sizeof (EAS_PCM) * m_channels;
    period_bytes = pa_usec_to_bytes(m_bufferTime * 1000, &samplespec);
    bufattr.maxlength = (int32_t)-1;
    bufattr.tlength = period_bytes;
    bufattr.minreq = (int32_t)-1;
    bufattr.prebuf = (int32_t)-1;
    bufattr.fragsize = (int32_t)-1;

    m_pulseHandle = pa_simple_new (server, "SonivoxEAS", PA_STREAM_PLAYBACK,
                    device, "Synthesizer output", &samplespec,
                    nullptr, /* pa_channel_map */
                    &bufattr,
                    &err);

    if (!m_pulseHandle)
    {
      m_diagnostics << "Failed to create PulseAudio connection";
      m_status = false;
    }
    //qDebug() << Q_FUNC_INFO << "period_bytes=" << period_bytes;
}

void
SynthRenderer::uninitEAS()
{
    EAS_RESULT eas_res;
    if (m_easData != nullptr && m_streamHandle != nullptr) {
      eas_res = EAS_CloseMIDIStream(m_easData, m_streamHandle);
      if (eas_res != EAS_SUCCESS) {
          m_diagnostics << QString("EAS_CloseMIDIStream error: %1").arg( eas_res );
      }
      eas_res = EAS_Shutdown(m_easData);
      if (eas_res != EAS_SUCCESS) {
          m_diagnostics << QString("EAS_Shutdown error: %1").arg( eas_res );
      }
      m_streamHandle = nullptr;
      m_easData = nullptr;
    }
    //qDebug() << Q_FUNC_INFO;
}

void
SynthRenderer::uninitPulse()
{
    if (m_pulseHandle != nullptr) {
        pa_simple_free(m_pulseHandle);
        m_pulseHandle = nullptr;
    }
    //qDebug() << Q_FUNC_INFO;
}

SynthRenderer::~SynthRenderer()
{
    //qDebug() << Q_FUNC_INFO;
}

void
SynthRenderer::initialize(QSettings *settings)
{
    //qDebug() << Q_FUNC_INFO;

    settings->beginGroup(QSTR_PREFERENCES);
    m_bufferTime = settings->value(QSTR_BUFFERTIME, DEF_BUFFERTIME).toInt();
    int reverbType = settings->value(QSTR_REVERBTYPE, DEF_REVERBTYPE).toInt();
    int reverbAmt = settings->value(QSTR_REVERBAMT, DEF_REVERBAMT).toInt();
    int chorusType = settings->value(QSTR_CHORUSTYPE, DEF_CHORUSTYPE).toInt();
    int chorusAmt = settings->value(QSTR_CHORUSAMT, DEF_CHORUSAMT).toInt();
    m_soundfont = settings->value(QSTR_SOUNDFONT, QString()).toString();
    settings->endGroup();

    initEAS();
    initSoundfont();
    initReverb(reverbType);
    setReverbWet(reverbAmt);
    initChorus(chorusType);
    setChorusLevel(chorusAmt);
}

bool
SynthRenderer::stopped()
{
    QReadLocker locker(&m_mutex);
    return m_Stopped;
}

void
SynthRenderer::stop()
{
    QWriteLocker locker(&m_mutex);
    //qDebug() << Q_FUNC_INFO;
    uninitEAS();
    m_Stopped = true;
}

void
SynthRenderer::run()
{
    int pa_err;
    unsigned char data[1024];
    //qDebug() << Q_FUNC_INFO << "started";
    try {
        initPulse();
        //qDebug() << Q_FUNC_INFO << "m_status:" << m_status;
        m_Stopped = false;
        if (m_rendering != nullptr) {
            m_rendering->wakeAll();
        }
        while (!stopped() && m_status) {
            EAS_RESULT eas_res;
            EAS_I32 numGen = 0;
            size_t bytes = 0;
            QCoreApplication::sendPostedEvents();
            if (m_easData != nullptr)
            {
                EAS_PCM *buffer = (EAS_PCM *) data;
                eas_res = EAS_Render(m_easData, buffer, m_bufferSize, &numGen);
                if (eas_res != EAS_SUCCESS) {
                    m_diagnostics << QString("EAS_Render error: %1").arg(eas_res);
                }
                bytes += (size_t) numGen * sizeof(EAS_PCM) * m_channels;
                // hand over to pulseaudio the rendered buffer
                if (pa_simple_write (m_pulseHandle, data, bytes, &pa_err) < 0)
                {
                    m_diagnostics << QString("Error writing to PulseAudio connection: %1").arg(pa_err);
                }
            }
        }
        uninitPulse();
    } catch (...) {
        m_diagnostics << "Exception in rendering loop - exiting";
        m_status = false;
    }
    //qDebug() << Q_FUNC_INFO << "ended";
    Q_EMIT finished();
}

void
SynthRenderer::writeMIDIData(const QByteArray& message)
{
    EAS_RESULT eas_res = EAS_ERROR_ALREADY_STOPPED;
    if (m_easData != nullptr && m_streamHandle != nullptr)
    {
        if (message.length() > 0) {
            //qDebug() << Q_FUNC_INFO << message.toHex();
            eas_res = EAS_WriteMIDIStream(m_easData, m_streamHandle, (EAS_U8 *)message.data(), message.length());
            if (eas_res != EAS_SUCCESS) {
                m_diagnostics << QString("EAS_WriteMIDIStream error: %1").arg(eas_res);
            }
        }
    }
}

void SynthRenderer::initSoundfont()
{
    //qDebug() << Q_FUNC_INFO;
    if (!m_soundfont.isEmpty()) {
        for(int ch = 0; ch < MIDI_STD_CHANNELS; ++ch) {
            if (ch == MIDI_GM_STD_DRUM_CHANNEL) {
                sendMessage(MIDI_STATUS_CONTROLCHANGE + ch, MIDI_CONTROL_MSB_BANK_SELECT, 0);
                sendMessage(MIDI_STATUS_CONTROLCHANGE + ch, MIDI_CONTROL_LSB_BANK_SELECT, 127);
            } else {
                sendMessage(MIDI_STATUS_CONTROLCHANGE + ch, MIDI_CONTROL_MSB_BANK_SELECT, 0);
                sendMessage(MIDI_STATUS_CONTROLCHANGE + ch, MIDI_CONTROL_LSB_BANK_SELECT, 0);
            }
            sendMessage(MIDI_STATUS_PROGRAMCHANGE + ch, 0);
        }
    }
}

bool SynthRenderer::getStatus() const
{
    return m_status;
}

QStringList SynthRenderer::getDiagnostics() const
{
    return m_diagnostics;
}

void SynthRenderer::setCondition(QWaitCondition *cond)
{
    m_rendering = cond;
}

QString SynthRenderer::getLibVersion()
{
    quint8 v1, v2, v3, v4;
    v1 = (m_libVersion >> 24) & 0xff;
    v2 = (m_libVersion >> 16) & 0xff;
    v3 = (m_libVersion >> 8) & 0xff;
    v4 = m_libVersion & 0xff;
    QVersionNumber vn{v1, v2, v3, v4};
    return vn.toString();
}

QString SynthRenderer::getSoundFont()
{
    return m_soundfont;
}

void SynthRenderer::writeSettings(QSettings *settings)
{
    if (settings != nullptr) {
        settings->beginGroup(QSTR_PREFERENCES);
        settings->setValue(QSTR_BUFFERTIME, m_bufferTime);
        settings->setValue(QSTR_REVERBTYPE, m_reverbType);
        settings->setValue(QSTR_REVERBAMT, m_reverbAmt);
        settings->setValue(QSTR_CHORUSTYPE, m_chorusType);
        settings->setValue(QSTR_CHORUSAMT, m_chorusAmt);
        settings->setValue(QSTR_SOUNDFONT, m_soundfont);
        settings->endGroup();
    }
}

void
SynthRenderer::initReverb(int reverb_type)
{
    EAS_RESULT eas_res;
    EAS_BOOL sw = EAS_TRUE;
    if ( reverb_type >= EAS_PARAM_REVERB_LARGE_HALL && reverb_type <= EAS_PARAM_REVERB_ROOM ) {
        sw = EAS_FALSE;
        eas_res = EAS_SetParameter(m_easData, EAS_MODULE_REVERB, EAS_PARAM_REVERB_PRESET, (EAS_I32) reverb_type);
        if (eas_res != EAS_SUCCESS) {
            m_diagnostics << QString("EAS_SetParameter error: %1").arg(eas_res);
        } else {
            m_reverbType = reverb_type;
        }
    }
    eas_res = EAS_SetParameter(m_easData, EAS_MODULE_REVERB, EAS_PARAM_REVERB_BYPASS, sw);
    if (eas_res != EAS_SUCCESS) {
        m_diagnostics << QString("EAS_SetParameter error: %1").arg(eas_res);
    }
}

void
SynthRenderer::initChorus(int chorus_type)
{
    EAS_RESULT eas_res;
    EAS_BOOL sw = EAS_TRUE;
    if (chorus_type >= EAS_PARAM_CHORUS_PRESET1 && chorus_type <= EAS_PARAM_CHORUS_PRESET4 ) {
        sw = EAS_FALSE;
        eas_res = EAS_SetParameter(m_easData, EAS_MODULE_CHORUS, EAS_PARAM_CHORUS_PRESET, (EAS_I32) chorus_type);
        if (eas_res != EAS_SUCCESS) {
            m_diagnostics << QString("EAS_SetParameter error: %1").arg(eas_res);
        } else {
            m_chorusType = chorus_type;
        }
    }
    eas_res = EAS_SetParameter(m_easData, EAS_MODULE_CHORUS, EAS_PARAM_CHORUS_BYPASS, sw);
    if (eas_res != EAS_SUCCESS) {
        m_diagnostics << QString("EAS_SetParameter error: %1").arg(eas_res);
    }
}

void
SynthRenderer::setReverbWet(int amount)
{
    EAS_RESULT eas_res = EAS_SetParameter(m_easData, EAS_MODULE_REVERB, EAS_PARAM_REVERB_WET, (EAS_I32) amount);
    if (eas_res != EAS_SUCCESS) {
        m_diagnostics << QString("EAS_SetParameter error: %1").arg(eas_res);
    } else {
        m_reverbAmt = amount;
    }
}

void
SynthRenderer::setChorusLevel(int amount)
{
    EAS_RESULT eas_res = EAS_SetParameter(m_easData, EAS_MODULE_CHORUS, EAS_PARAM_CHORUS_LEVEL, (EAS_I32) amount);
    if (eas_res != EAS_SUCCESS) {
        m_diagnostics << QString("EAS_SetParameter error: %1").arg(eas_res);
    } else {
        m_chorusAmt = amount;
    }
}

void
SynthRenderer::sendMessage(int m0)
{
    QByteArray m;
    m.resize(1);
    m[0] = m0;
    writeMIDIData(m);
}

void
SynthRenderer::sendMessage(int m0, int m1)
{
    QByteArray m;
    m.resize(2);
    m[0] = m0;
    m[1] = m1;
    writeMIDIData(m);
}

void
SynthRenderer::sendMessage(int m0, int m1, int m2)
{
    QByteArray m;
    m.resize(3);
    m[0] = m0;
    m[1] = m1;
    m[2] = m2;
    writeMIDIData(m);
}

MIDIConnection
SynthRenderer::connection()
{
    if (stopped()) {
        return MIDIConnection();
    } else {
        return MIDIConnection(QSTR_SONIVOXEAS, QSTR_SONIVOXEAS);
    }
}

void
SynthRenderer::setBufferTime(int milliseconds)
{
    //qDebug() << Q_FUNC_INFO << milliseconds;
    m_bufferTime = milliseconds;
}

} // namespace rt
} // namespace drumstick
