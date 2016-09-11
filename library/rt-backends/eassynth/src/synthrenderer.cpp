/*
    Sonivox EAS Synthesizer for Qt applications
    Copyright (C) 2016, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <QObject>
#include <QString>
#include <QCoreApplication>
#include <QTextStream>
#include <QtDebug>
#include <QReadLocker>
#include <QWriteLocker>
#include <eas_reverb.h>
#include <eas_chorus.h>
#include <pulse/simple.h>
#include "synthrenderer.h"
#include "drumstickcommon.h"

namespace drumstick {
namespace rt {

const QString QSTR_PREFERENCES("SonivoxEAS");
const QString QSTR_BUFFERTIME("BufferTime");
const QString QSTR_REVERBTYPE("ReverbType");
const QString QSTR_REVERBAMT("ReverbAmt");
const QString QSTR_CHORUSTYPE("ChorusType");
const QString QSTR_CHORUSAMT("ChorusAmt");

SynthRenderer::SynthRenderer(QObject *parent) : QObject(parent),
    m_Stopped(true),
    m_bufferTime(60)
{
    initEAS();
}

void
SynthRenderer::initEAS()
{
    /* SONiVOX EAS initialization */
    EAS_RESULT eas_res;
    EAS_DATA_HANDLE dataHandle;
    EAS_HANDLE handle;

    const S_EAS_LIB_CONFIG *easConfig = EAS_Config();
    if (easConfig == 0) {
        qCritical() << "EAS_Config returned null";
        return;
    }

    eas_res = EAS_Init(&dataHandle);
    if (eas_res != EAS_SUCCESS) {
      qCritical() << "EAS_Init error: " << eas_res;
      return;
    }

    eas_res = EAS_OpenMIDIStream(dataHandle, &handle, NULL);
    if (eas_res != EAS_SUCCESS) {
      qCritical() << "EAS_OpenMIDIStream error: " << eas_res;
      EAS_Shutdown(dataHandle);
      return;
    }

    m_easData = dataHandle;
    m_streamHandle = handle;
    assert(m_streamHandle != 0);
    m_sampleRate = easConfig->sampleRate;
    m_bufferSize = easConfig->mixBufferSize;
    m_channels = easConfig->numChannels;
    qDebug() << Q_FUNC_INFO << "EAS bufferSize=" << m_bufferSize << " sampleRate=" << m_sampleRate << " channels=" << m_channels;
}

void
SynthRenderer::initPulse()
{
    pa_sample_spec samplespec;
    pa_buffer_attr bufattr;
    int period_bytes;
    char *server = 0;
    char *device = 0;
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
                    NULL, /* pa_channel_map */
                    &bufattr,
                    &err);

    if (!m_pulseHandle)
    {
      qCritical() << "Failed to create PulseAudio connection";
    }
    qDebug() << Q_FUNC_INFO << "period_bytes=" << period_bytes;
}

void
SynthRenderer::uninitEAS()
{
    EAS_RESULT eas_res;
    if (m_easData != 0 && m_streamHandle != 0) {
      eas_res = EAS_CloseMIDIStream(m_easData, m_streamHandle);
      if (eas_res != EAS_SUCCESS) {
          qWarning() << "EAS_CloseMIDIStream error: " << eas_res;
      }
      eas_res = EAS_Shutdown(m_easData);
      if (eas_res != EAS_SUCCESS) {
          qWarning() << "EAS_Shutdown error: " << eas_res;
      }
      m_streamHandle = 0;
      m_easData = 0;
    }
    qDebug() << Q_FUNC_INFO;
}

void
SynthRenderer::uninitPulse()
{
    if (m_pulseHandle != 0) {
        pa_simple_free(m_pulseHandle);
        m_pulseHandle = 0;
    }
    qDebug() << Q_FUNC_INFO;
}

SynthRenderer::~SynthRenderer()
{
    uninitEAS();
}

void
SynthRenderer::initialize(QSettings *settings)
{
    settings->beginGroup(QSTR_PREFERENCES);
    m_bufferTime = settings->value(QSTR_BUFFERTIME, 60).toInt();
    int reverbType = settings->value(QSTR_REVERBTYPE, EAS_PARAM_REVERB_HALL).toInt();
    int reverbAmt = settings->value(QSTR_REVERBAMT, 25800).toInt();
    int chorusType = settings->value(QSTR_CHORUSTYPE, -1).toInt();
    int chorusAmt = settings->value(QSTR_CHORUSAMT, 0).toInt();
    settings->endGroup();
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
    qDebug() << Q_FUNC_INFO;
    m_Stopped = true;
}

void
SynthRenderer::run()
{
    int pa_err;
    unsigned char data[1024];
    qDebug() << Q_FUNC_INFO << "started";
    try {
        initPulse();
        m_Stopped = false;
        while (!stopped()) {
            EAS_RESULT eas_res;
            EAS_I32 numGen = 0;
            size_t bytes = 0;
            QCoreApplication::sendPostedEvents();
            if (m_easData != 0)
            {
                EAS_PCM *buffer = (EAS_PCM *) data;
                eas_res = EAS_Render(m_easData, buffer, m_bufferSize, &numGen);
                if (eas_res != EAS_SUCCESS) {
                    qWarning() << "EAS_Render error:" << eas_res;
                }
                bytes += (size_t) numGen * sizeof(EAS_PCM) * m_channels;
                // hand over to pulseaudio the rendered buffer
                if (pa_simple_write (m_pulseHandle, data, bytes, &pa_err) < 0)
                {
                    qWarning() << "Error writing to PulseAudio connection:" << pa_err;
                }
            }
        }
        uninitPulse();
    } catch (const SequencerError& err) {
        qWarning() << "SequencerError exception. Error code: " << err.code()
                   << " (" << err.qstrError() << ")";
        qWarning() << "Location: " << err.location();
    }
    qDebug() << Q_FUNC_INFO << "ended";
    emit finished();
}

void
SynthRenderer::writeMIDIData(const QByteArray& message)
{
    EAS_RESULT eas_res = EAS_ERROR_ALREADY_STOPPED;
    if (m_easData != 0 && m_streamHandle != 0)
    {
        //count = m_codec->decode((unsigned char *)&buffer, sizeof(buffer), ev->getHandle());
        if (message.length() > 0) {
            //qDebug() << Q_FUNC_INFO << message.toHex();
            eas_res = EAS_WriteMIDIStream(m_easData, m_streamHandle, (EAS_U8 *)message.data(), message.length());
            if (eas_res != EAS_SUCCESS) {
                qWarning() << "EAS_WriteMIDIStream error: " << eas_res;
            }
        }
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
            qWarning() << "EAS_SetParameter error:" << eas_res;
        }
    }
    eas_res = EAS_SetParameter(m_easData, EAS_MODULE_REVERB, EAS_PARAM_REVERB_BYPASS, sw);
    if (eas_res != EAS_SUCCESS) {
        qWarning() << "EAS_SetParameter error: " << eas_res;
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
            qWarning() << "EAS_SetParameter error:" << eas_res;
        }
    }
    eas_res = EAS_SetParameter(m_easData, EAS_MODULE_CHORUS, EAS_PARAM_CHORUS_BYPASS, sw);
    if (eas_res != EAS_SUCCESS) {
        qWarning() << "EAS_SetParameter error:" << eas_res;
    }
}

void
SynthRenderer::setReverbWet(int amount)
{
    EAS_RESULT eas_res = EAS_SetParameter(m_easData, EAS_MODULE_REVERB, EAS_PARAM_REVERB_WET, (EAS_I32) amount);
    if (eas_res != EAS_SUCCESS) {
        qWarning() << "EAS_SetParameter error:" << eas_res;
    }
}

void
SynthRenderer::setChorusLevel(int amount)
{
    EAS_RESULT eas_res = EAS_SetParameter(m_easData, EAS_MODULE_CHORUS, EAS_PARAM_CHORUS_LEVEL, (EAS_I32) amount);
    if (eas_res != EAS_SUCCESS) {
        qWarning() << "EAS_SetParameter error:" << eas_res;
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

QString
SynthRenderer::connection()
{
    if (stopped()) {
        return QString();
    } else {
        return QSTR_SONIVOXEAS;
    }
}

void
SynthRenderer::setBufferTime(int milliseconds)
{
    qDebug() << Q_FUNC_INFO << milliseconds;
    m_bufferTime = milliseconds;
}

}}
