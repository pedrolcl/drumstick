/*
    Virtual Piano test using the MIDI Sequencer C++ library 
    Copyright (C) 2006-2015, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include "connections.h"
#include "networksettingsdialog.h"
#include "fluidsettingsdialog.h"

Connections::Connections(QWidget *parent)
    : QDialog(parent),
      m_advanced(false),
      m_thru(false),
      m_settingsChanged(false),
      m_midiIn(0),
      m_midiOut(0)
{
    ui.setupUi(this);
    connect(ui.m_advanced, SIGNAL(clicked(bool)), SLOT(clickedAdvanced(bool)));
    connect(ui.m_inputBackends, SIGNAL(currentIndexChanged(QString)), SLOT(refreshInputs(QString)));
    connect(ui.m_outputBackends, SIGNAL(currentIndexChanged(QString)), SLOT(refreshOutputs(QString)));
    connect(ui.btnInputDriverCfg, &QToolButton::clicked, this, &Connections::configureInputDriver);
    connect(ui.btnOutputDriverCfg, &QToolButton::clicked, this, &Connections::configureOutputDriver);
    ui.m_advanced->setChecked(m_advanced);
    ui.m_thru->setChecked(m_thru);
}

void Connections::setInputs(QList<MIDIInput *> ins)
{
    ui.m_inputBackends->disconnect();
    ui.m_inputBackends->clear();
    foreach(MIDIInput *i, ins) {
        ui.m_inputBackends->addItem(i->backendName(), qVariantFromValue((void *) i));
    }
    connect(ui.m_inputBackends, SIGNAL(currentIndexChanged(QString)), SLOT(refreshInputs(QString)));
}

void Connections::setOutputs(QList<MIDIOutput *> outs)
{
    ui.m_outputBackends->disconnect();
    foreach(MIDIOutput *o, outs) {
        ui.m_outputBackends->addItem(o->backendName(), qVariantFromValue((void *) o));
    }
    connect(ui.m_outputBackends, SIGNAL(currentIndexChanged(QString)), SLOT(refreshOutputs(QString)));
}

void Connections::accept()
{
    QString conn;
    QSettings settings;
    m_advanced = ui.m_advanced->isChecked();
    m_thru = ui.m_thru->isChecked();
    if (m_midiOut != 0) {
        conn = ui.m_outputPorts->currentText();
        if (conn != m_midiOut->currentConnection() || m_settingsChanged) {
            m_midiOut->close();
            if (!conn.isEmpty()) {
                m_midiOut->initialize(&settings);
                m_midiOut->open(conn);
            }
        }
    }
    if (m_midiIn != 0) {
        conn = ui.m_inputPorts->currentText();
        if (conn != m_midiIn->currentConnection() || m_settingsChanged) {
            m_midiIn->close();
            if (!conn.isEmpty()) {
                m_midiIn->initialize(&settings);
                m_midiIn->open(conn);
            }
        }
        if (m_midiOut != 0) {
            m_midiIn->setMIDIThruDevice(m_midiOut);
            m_midiIn->enableMIDIThru(m_thru);
        }
    }
    m_settingsChanged = false;
    QDialog::accept();
}

void Connections::refresh()
{
    m_advanced = ui.m_advanced->isChecked();
    if (m_midiIn != 0) {
        ui.m_inputBackends->setCurrentText(m_midiIn->backendName());
        refreshInputs(m_midiIn->backendName());
    }
    if (m_midiOut != 0) {
        ui.m_outputBackends->setCurrentText(m_midiOut->backendName());
        refreshOutputs(m_midiOut->backendName());
    }
}

void Connections::refreshInputs(QString id)
{
    ui.btnInputDriverCfg->setEnabled(id == "Network");
    if (m_midiIn != 0 && m_midiIn->backendName() != id) {
        m_midiIn->close();
        int idx = ui.m_inputBackends->findText(id, Qt::MatchStartsWith);
        if (idx > -1) {
            m_midiIn = (MIDIInput *) ui.m_inputBackends->itemData(idx).value<void *>();
        } else {
            m_midiIn = 0;
        }
    }
    ui.m_inputPorts->clear();
    if (m_midiIn != 0) {
        ui.m_inputPorts->addItem(QString());
        ui.m_inputPorts->addItems(m_midiIn->connections(m_advanced));
        ui.m_inputPorts->setCurrentText(m_midiIn->currentConnection());
    }
}

void Connections::refreshOutputs(QString id)
{
    ui.btnOutputDriverCfg->setEnabled(id == "Network" || id == "FluidSynth");
    if (m_midiOut != 0 && m_midiOut->backendName() != id) {
        m_midiOut->close();
        int idx = ui.m_outputBackends->findText(id, Qt::MatchStartsWith);
        if (idx > -1) {
            m_midiOut = (MIDIOutput *) ui.m_outputBackends->itemData(idx).value<void *>();
        } else {
            m_midiOut = 0;
        }
    }
    ui.m_outputPorts->clear();
    if (m_midiOut != 0) {
        ui.m_outputPorts->addItems(m_midiOut->connections(m_advanced));
        ui.m_outputPorts->setCurrentText(m_midiOut->currentConnection());
    }
}

void Connections::setAdvanced(bool value)
{
    ui.m_advanced->setChecked(value);
    refresh();
}

void Connections::clickedAdvanced(bool value)
{
    m_advanced = value;
    refresh();
}

void Connections::setMidiThru(bool value)
{
    m_thru = value;
    ui.m_thru->setChecked(value);
}

bool Connections::advanced()
{
    return ui.m_advanced->isChecked();
}

bool Connections::midiThru()
{
    return ui.m_thru->isChecked();
}

void Connections::configureInputDriver()
{
    QString driver = ui.m_inputBackends->currentText();
    if (driver == "Network") {
        NetworkSettingsDialog dlg(this);
        m_settingsChanged |= (dlg.exec() == QDialog::Accepted);
    }
}

void Connections::configureOutputDriver()
{
    QString driver = ui.m_outputBackends->currentText();
    if (driver == "Network") {
        NetworkSettingsDialog dlg(this);
        m_settingsChanged |= (dlg.exec() == QDialog::Accepted);
    } else if (driver == "FluidSynth") {
        FluidSettingsDialog dlg(this);
        m_settingsChanged |= (dlg.exec() == QDialog::Accepted);
    }
}
