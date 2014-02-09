/*
    Virtual Piano test using the MIDI Sequencer C++ library 
    Copyright (C) 2006-2013, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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
#include <QDebug>
#include "connections.h"

Connections::Connections(QWidget *parent)
    : QDialog(parent),
      m_midiIn(0),
      m_midiOut(0)
{
    ui.setupUi(this);
    connect(ui.m_advanced, SIGNAL(toggled(bool)), SLOT(refresh()));
    connect(ui.m_inputBackends, SIGNAL(currentIndexChanged(QString)), SLOT(refreshInputs(QString)));
    connect(ui.m_outputBackends, SIGNAL(currentIndexChanged(QString)), SLOT(refreshOutputs(QString)));
}

void Connections::setInputs(QList<MIDIInput *> ins)
{
    m_inputs.clear();
    foreach(MIDIInput *i, ins) {
        m_inputs.insert(i->backendName(), i);
    }
}

void Connections::setOutputs(QList<MIDIOutput *> outs)
{
    m_outputs.clear();
    foreach(MIDIOutput *o, outs) {
        m_outputs.insert(o->backendName(), o);
    }
}

void Connections::accept()
{
    QString conn;
    if (m_midiOut != 0) {
        conn = ui.m_outputPorts->currentText();
        if (conn != m_midiOut->currentConnection()) {
            m_midiOut->close();
            if (!conn.isEmpty())
                m_midiOut->open(conn);
        }
    }
    if (m_midiIn != 0) {
        conn = ui.m_inputPorts->currentText();
        if (conn != m_midiIn->currentConnection()) {
            m_midiIn->close();
            if (!conn.isEmpty())
                m_midiIn->open(conn);
        }
        m_midiIn->enableMIDIThru(ui.m_thru->isChecked());
        m_midiIn->setMIDIThruDevice(m_midiOut);
    }
    QDialog::accept();
}

void Connections::refresh()
{
    ui.m_inputBackends->disconnect();
    ui.m_outputBackends->disconnect();
    ui.m_inputBackends->clear();
    ui.m_outputBackends->clear();
    foreach(MIDIInput *i, m_inputs.values()) {
        ui.m_inputBackends->addItem(i->backendName());
    }
    foreach(MIDIOutput *o, m_outputs.values()) {
        ui.m_outputBackends->addItem(o->backendName());
    }
    if (m_midiIn != 0) {
        ui.m_inputBackends->setCurrentText(m_midiIn->backendName());
        refreshInputs(m_midiIn->backendName());
    }
    if (m_midiOut != 0) {
        ui.m_outputBackends->setCurrentText(m_midiOut->backendName());
        refreshOutputs(m_midiOut->backendName());
    }
    connect(ui.m_inputBackends, SIGNAL(currentIndexChanged(QString)), SLOT(refreshInputs(QString)));
    connect(ui.m_outputBackends, SIGNAL(currentIndexChanged(QString)), SLOT(refreshOutputs(QString)));
}

void Connections::refreshInputs(QString id)
{
    qDebug() << Q_FUNC_INFO << id;
    bool advanced = ui.m_advanced->isChecked();
    if (m_midiIn != 0) {
        m_midiIn->close();
    }
    if (m_inputs.contains(id))
        m_midiIn = m_inputs[id];
    else
        m_midiIn = 0;
    ui.m_inputPorts->clear();
    if (m_midiIn != 0) {
        ui.m_inputPorts->addItem(QString());
        ui.m_inputPorts->addItems(m_midiIn->connections(advanced));
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
        ui.m_inputPorts->setCurrentIndex(ui.m_inputs->findText(m_midiIn->currentConnection()));
#else
        ui.m_inputPorts->setCurrentText(m_midiIn->currentConnection());
#endif
    }
}

void Connections::refreshOutputs(QString id)
{
    qDebug() << Q_FUNC_INFO << id;
    bool advanced = ui.m_advanced->isChecked();
    if (m_midiOut != 0) {
        m_midiOut->close();
    }
    if (m_outputs.contains(id))
        m_midiOut = m_outputs[id];
    else
        m_midiOut = 0;
    ui.m_outputPorts->clear();
    if (m_midiOut != 0) {
        ui.m_outputPorts->addItems(m_midiOut->connections(advanced));
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
        ui.m_outputPorts->setCurrentIndex(ui.m_outputs->findText(m_midiOut->currentConnection()));
#else
        ui.m_outputPorts->setCurrentText(m_midiOut->currentConnection());
#endif
    }
}
