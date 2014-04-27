/*
    Drumstick RT (realtime MIDI In/Out)
    Copyright (C) 2009-2014 Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include <QObject>
#include <QFile>
#include <QDir>
#include <QDebug>

#include "ossinput_p.h"
#include "ossinput.h"
#include "ossoutput.h"

namespace drumstick {
namespace rt {

static QString DEFAULT_PUBLIC_NAME(QLatin1String("MIDI In"));

OSSInputPrivate::OSSInputPrivate(QObject *parent) : QObject(parent),
    m_inp(qobject_cast<OSSInput *>(parent)),
    m_out(0),
    m_device(0),
    m_notifier(0),
    m_thruEnabled(false),
    m_advanced(false),
    m_publicName(DEFAULT_PUBLIC_NAME)
{
    reloadDeviceList();
}

void OSSInputPrivate::reloadDeviceList(bool advanced)
{
    QDir dir("/dev");
    QStringList filters;
    m_advanced = advanced;
    filters << "dmmidi*" << "admmidi*";
    if (advanced) {
        filters << "midi*" << "amidi*";
    }
    dir.setNameFilters(filters);
    dir.setFilter(QDir::System);
    dir.setSorting(QDir::Name);
    m_inputDevices.clear();
    QFileInfoList listInfo = dir.entryInfoList();
    foreach(const QFileInfo &info, listInfo) {
        m_inputDevices << info.absoluteFilePath();
    }
}

void OSSInputPrivate::open(QString portName)
{
    QFile *f = new QFile(portName);
    m_currentInput = portName;
    m_device = f;
    m_device->open( QIODevice::ReadOnly | QIODevice::Unbuffered );
    m_notifier = new QSocketNotifier(f->handle(), QSocketNotifier::Read);
    m_buffer.clear();
    connect(m_notifier, SIGNAL(activated(int)), this, SLOT(processIncomingMessages(int)));
    qDebug() << Q_FUNC_INFO << portName;
}

void OSSInputPrivate::close()
{
    if (m_device != 0) {
        m_device->close();
        delete m_notifier;
        delete m_device;
        m_device = 0;
    }
    m_currentInput.clear();
}

void OSSInputPrivate::parse()
{
    uchar status = static_cast<unsigned>(m_buffer.at(0));
    if (status < MIDI_STATUS_SYSEX) { // channel message
        int chan = status & MIDI_CHANNEL_MASK;
        status = status & MIDI_STATUS_MASK;
        switch(status) {
        case MIDI_STATUS_NOTEOFF: {
            if (m_buffer.length() < 3)
                return;
            int m1 = static_cast<unsigned>(m_buffer.at(1));
            int m2 = static_cast<unsigned>(m_buffer.at(2));
            if(m_out != 0 && m_thruEnabled) {
                m_out->sendNoteOff(chan, m1, m2);
            }
            emit m_inp->midiNoteOff(chan, m1, m2);
            break; }
        case MIDI_STATUS_NOTEON: {
            if (m_buffer.length() < 3)
                return;
            int m1 = static_cast<unsigned>(m_buffer.at(1));
            int m2 = static_cast<unsigned>(m_buffer.at(2));
            if(m_out != 0 && m_thruEnabled) {
                m_out->sendNoteOn(chan, m1, m2);
            }
            emit m_inp->midiNoteOn(chan, m1, m2);
            break; }
        case MIDI_STATUS_KEYPRESURE: {
            if (m_buffer.length() < 3)
                return;
            int m1 = static_cast<unsigned>(m_buffer.at(1));
            int m2 = static_cast<unsigned>(m_buffer.at(2));
            if(m_out != 0 && m_thruEnabled) {
                m_out->sendKeyPressure(chan, m1, m2);
            }
            emit m_inp->midiKeyPressure(chan, m1, m2);
            break; }
        case MIDI_STATUS_CONTROLCHANGE: {
            if (m_buffer.length() < 3)
                return;
            int m1 = static_cast<unsigned>(m_buffer.at(1));
            int m2 = static_cast<unsigned>(m_buffer.at(2));
            if(m_out != 0 && m_thruEnabled) {
                m_out->sendController(chan, m1, m2);
            }
            emit m_inp->midiController(chan, m1, m2);
            break; }
        case MIDI_STATUS_PROGRAMCHANGE: {
            if (m_buffer.length() < 2)
                return;
            int m1 = static_cast<unsigned>(m_buffer.at(1));
            if(m_out != 0 && m_thruEnabled) {
                m_out->sendProgram(chan, m1);
            }
            emit m_inp->midiProgram(chan, m1);
            break; }
        case MIDI_STATUS_CHANNELPRESSURE: {
            if (m_buffer.length() < 2)
                return;
            int m1 = static_cast<unsigned>(m_buffer.at(1));
            if(m_out != 0 && m_thruEnabled) {
                m_out->sendChannelPressure(chan, m1);
            }
            emit m_inp->midiChannelPressure(chan, m1);
            break; }
        case MIDI_STATUS_PITCHBEND: {
            if (m_buffer.length() < 3)
                return;
            int lo = static_cast<unsigned>(m_buffer.at(1));
            int hi = static_cast<unsigned>(m_buffer.at(2));
            int v = lo + hi * 0x80 - 0x2000;
            if(m_out != 0 && m_thruEnabled) {
                m_out->sendPitchBend(chan, v);
            }
            emit m_inp->midiPitchBend(chan, v);
            break; }
        }
        qDebug() << Q_FUNC_INFO << m_buffer.toHex();
        m_buffer.clear();
    } else { // system message
        if (status == MIDI_STATUS_SYSEX) {
            int p = m_buffer.at(m_buffer.length() - 1);
            if (p != MIDI_STATUS_ENDSYSEX)
                return;
            if(m_out != 0 && m_thruEnabled) {
                m_out->sendSysex(m_buffer);
            }
            emit m_inp->midiSysex(m_buffer);
            qDebug() << Q_FUNC_INFO << m_buffer.toHex();
            m_buffer.clear();
        }
    }
}

void OSSInputPrivate::processIncomingMessages(int)
{
    char ch;
    m_device->getChar(&ch);
    uchar uch = static_cast<unsigned>(ch);
    switch (uch) {
        /*case 0xF1:
        case 0xF2:
        case 0xF3:
        case 0xF4:
        case 0xF5:
        case 0xF6:
            if(m_out != 0 && m_thruEnabled) {
                m_out->sendSystemMsg(uch);
            }
            emit m_inp->midiSystemCommon(uch);
            break;*/
        case 0xF8:
        case 0xF9:
        case 0xFA:
        case 0xFB:
        case 0xFC:
        case 0xFD:
        case 0xFE:
        case 0xFF:
            if(m_out != 0 && m_thruEnabled) {
                m_out->sendSystemMsg(uch);
            }
            emit m_inp->midiSystemRealtime(uch);
            break;
        default:
            m_buffer.append(ch);
            parse();
    }
}

}}
