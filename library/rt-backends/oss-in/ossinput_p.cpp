/*
    Drumstick RT (realtime MIDI In/Out)
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

#include <QObject>
#include <QFile>
#include <QDir>
#include <QDebug>

#include "ossinput_p.h"
#include "ossinput.h"

namespace drumstick {
namespace rt {

static QString DEFAULT_PUBLIC_NAME(QLatin1String("MIDI In"));

OSSInputPrivate::OSSInputPrivate(QObject *parent) : QObject(parent),
    m_inp(qobject_cast<OSSInput *>(parent)),
    m_out(0),
    m_device(0),
    m_notifier(0),
    m_parser(0),
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
    m_parser = new MIDIParser(m_inp);
    m_buffer.clear();
    connect(m_notifier, SIGNAL(activated(int)), this, SLOT(processIncomingMessages(int)));
    //qDebug() << Q_FUNC_INFO << portName;
}

void OSSInputPrivate::close()
{
    if (m_device != 0) {
        m_device->close();
        delete m_notifier;
        delete m_device;
        delete m_parser;
        m_device = 0;
        m_parser = 0;
    }
    m_currentInput.clear();
}

void OSSInputPrivate::setMIDIThruDevice(MIDIOutput* device)
{
    m_out = device;
    if (m_parser != 0) {
        m_parser->setMIDIThruDevice(device);
    }
}

void OSSInputPrivate::processIncomingMessages(int)
{
    char ch;
    m_device->getChar(&ch);
    if (m_parser != 0) {
        uchar uch = static_cast<unsigned>(ch);
        m_parser->parse(uch);
    }
}

}}
