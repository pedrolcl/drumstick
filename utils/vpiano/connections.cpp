/*
    Virtual Piano test using the MIDI Sequencer C++ library 
    Copyright (C) 2006-2020, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "connections.h"
#include "vpianosettings.h"
#include <drumstick/configurationdialogs.h>
#include <drumstick/settingsfactory.h>

using namespace drumstick::rt;

Connections::Connections(QWidget *parent)
    : QDialog(parent),
      m_settingsChanged(false),
      m_midiIn(nullptr),
      m_midiOut(nullptr)
{
    ui.setupUi(this);
    ui.m_advanced->setChecked(VPianoSettings::instance()->advanced());
    ui.m_thru->setChecked(VPianoSettings::instance()->midiThru());
    connect(ui.m_advanced, SIGNAL(clicked(bool)), SLOT(clickedAdvanced(bool)));
    connect(ui.m_inputBackends, SIGNAL(currentIndexChanged(QString)), SLOT(refreshInputs(QString)));
    connect(ui.m_outputBackends, SIGNAL(currentIndexChanged(QString)), SLOT(refreshOutputs(QString)));
    connect(ui.btnInputDriverCfg, &QToolButton::clicked, this, &Connections::configureInputDriver);
    connect(ui.btnOutputDriverCfg, &QToolButton::clicked, this, &Connections::configureOutputDriver);
}

void Connections::setInput(MIDIInput *in)
{
    m_midiIn = in;
}

void Connections::setOutput(MIDIOutput *out)
{
    m_midiOut = out;
}

void Connections::setInputs(QList<MIDIInput *> ins)
{
    ui.m_inputBackends->disconnect();
    ui.m_inputBackends->clear();
    foreach(MIDIInput *i, ins) {
        ui.m_inputBackends->addItem(i->backendName(), QVariant::fromValue(i));
    }
    connect(ui.m_inputBackends, SIGNAL(currentIndexChanged(QString)), SLOT(refreshInputs(QString)));
}

void Connections::setOutputs(QList<MIDIOutput *> outs)
{
    ui.m_outputBackends->disconnect();
    foreach(MIDIOutput *o, outs) {
        ui.m_outputBackends->addItem(o->backendName(), QVariant::fromValue(o));
    }
    connect(ui.m_outputBackends, SIGNAL(currentIndexChanged(QString)), SLOT(refreshOutputs(QString)));
}

MIDIInput *Connections::getInput()
{
    return m_midiIn;
}

MIDIOutput *Connections::getOutput()
{
    return m_midiOut;
}

void Connections::accept()
{
    MIDIConnection connOut, connIn;
    drumstick::widgets::SettingsFactory settings;
    VPianoSettings::instance()->setAdvanced(ui.m_advanced->isChecked());
    VPianoSettings::instance()->setMidiThru(ui.m_thru->isChecked());
    if (m_midiOut != nullptr) {
        connOut = ui.m_outputPorts->currentData().value<MIDIConnection>();
        if (connOut != m_midiOut->currentConnection() || m_settingsChanged) {
            m_midiOut->close();
            if (!connOut.first.isEmpty()) {
                m_midiOut->initialize(settings.getQSettings());
                m_midiOut->open(connOut);
            }
        }
        VPianoSettings::instance()->setLastOutputBackend(m_midiOut->backendName());
        VPianoSettings::instance()->setLastOutputConnection(connOut.first);
    }
    if (m_midiIn != nullptr) {
        connIn = ui.m_inputPorts->currentData().value<MIDIConnection>();
        if (connIn != m_midiIn->currentConnection() || m_settingsChanged) {
            m_midiIn->close();
            if (!connIn.first.isEmpty()) {
                m_midiIn->initialize(settings.getQSettings());
                m_midiIn->open(connIn);
            }
        }
        if (m_midiOut != nullptr) {
            m_midiIn->setMIDIThruDevice(m_midiOut);
            m_midiIn->enableMIDIThru(VPianoSettings::instance()->midiThru());
        }
        VPianoSettings::instance()->setLastInputBackend(m_midiIn->backendName());
        VPianoSettings::instance()->setLastInputConnection(connIn.first);
    }
    m_settingsChanged = false;
    QDialog::accept();
}

void Connections::refresh()
{
    if (m_midiIn != nullptr) {
        ui.m_inputBackends->setCurrentText(m_midiIn->backendName());
        refreshInputs(m_midiIn->backendName());
    }
    if (m_midiOut != nullptr) {
        ui.m_outputBackends->setCurrentText(m_midiOut->backendName());
        refreshOutputs(m_midiOut->backendName());
    }
}

void Connections::refreshInputs(QString id)
{
    ui.btnInputDriverCfg->setEnabled(id == "Network");
    if (m_midiIn != nullptr && m_midiIn->backendName() != id) {
        m_midiIn->close();
        int idx = ui.m_inputBackends->findText(id, Qt::MatchStartsWith);
        if (idx > -1) {
            m_midiIn = ui.m_inputBackends->itemData(idx).value<MIDIInput*>();
        } else {
            m_midiIn = nullptr;
        }
    }
    ui.m_inputPorts->clear();
    if (m_midiIn != nullptr) {
        ui.m_inputPorts->addItem(QString());
        const QList<MIDIConnection> conns = m_midiIn->connections(ui.m_advanced->isChecked());
        for (const MIDIConnection& conn : conns) {
            ui.m_inputPorts->addItem(conn.first, QVariant::fromValue(conn));
        }
        ui.m_inputPorts->setCurrentText(m_midiIn->currentConnection().first);
    }
}

void Connections::refreshOutputs(QString id)
{
    ui.btnOutputDriverCfg->setEnabled(id == "Network" ||
                                      id == "FluidSynth" ||
                                      id == "DLS Synth" ||
                                      id == "SonivoxEAS");
    if (m_midiOut != nullptr && m_midiOut->backendName() != id) {
        m_midiOut->close();
        int idx = ui.m_outputBackends->findText(id, Qt::MatchStartsWith);
        if (idx > -1) {
            m_midiOut = ui.m_outputBackends->itemData(idx).value<MIDIOutput*>();
        } else {
            m_midiOut = nullptr;
        }
    }
    ui.m_outputPorts->clear();
    if (m_midiOut != nullptr) {
        const QList<MIDIConnection> conns = m_midiOut->connections(ui.m_advanced->isChecked());
        for(const MIDIConnection& conn : conns) {
            ui.m_outputPorts->addItem(conn.first, QVariant::fromValue(conn));
        }
        ui.m_outputPorts->setCurrentText(m_midiOut->currentConnection().first);
    }
}

void Connections::clickedAdvanced(bool value)
{
    Q_UNUSED(value)
    refresh();
}

void Connections::configureInputDriver()
{
    QString driver = ui.m_inputBackends->currentText();
    if (driver == "Network") {
        m_settingsChanged |= drumstick::widgets::configureInputDriver(driver, this);
    }
}

void Connections::configureOutputDriver()
{
    QString driver = ui.m_outputBackends->currentText();
    if (driver == "Network" ||
        driver == "FluidSynth" ||
        driver == "SonivoxEAS" ||
        driver == "DLS Synth") {
        m_settingsChanged |= drumstick::widgets::configureOutputDriver(driver, this);
    }
}
