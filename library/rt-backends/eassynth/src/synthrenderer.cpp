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
#include <ao/ao.h>
#include "synthrenderer.h"
#include "drumstickcommon.h"

namespace drumstick {
namespace rt {

SynthRenderer::SynthRenderer(QObject *parent) : QObject(parent),
    m_Stopped(true)
{
    initEAS();
    initAudio();
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
SynthRenderer::initAudio()
{
    ao_sample_format format;
    ::memset(&format, 0, sizeof(format));
    format.bits = 16;
    format.channels = m_channels;
    format.rate = m_sampleRate;
    format.byte_format = AO_FMT_LITTLE;

    ao_initialize();
    int id = ao_default_driver_id();
//    ao_info* info = ao_driver_info(id);
//    if (info != 0) {
//        qDebug() << "libao driver type:" <<  info->type;
//        qDebug() << "libao driver short_name:" << info->short_name;
//        qDebug() << "libao driver name:" << info->name;
//        qDebug() << "libao driver comment:" << info->comment;
//    }
    m_aoDevice = ao_open_live(id, &format, 0);
    if (!m_aoDevice)
    {
      qCritical() << "Failed to create libAO audio device";
    }
    qDebug() << Q_FUNC_INFO;
}

SynthRenderer::~SynthRenderer()
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
    }
    ao_close(m_aoDevice);
    ao_shutdown();
    qDebug() << Q_FUNC_INFO;
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
    char data[1024];
    qDebug() << Q_FUNC_INFO << "started";
    try {
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
                // hand over to libao the rendered buffer
                if(ao_play(m_aoDevice, data, bytes) == 0) {
                    qWarning() << "Error writing audio";
                }
            }
        }
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

}}
