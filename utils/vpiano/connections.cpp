/*
    Virtual Piano test using the MIDI Sequencer C++ library 
    Copyright (C) 2006-2021, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include "connections.h"
#include "vpianosettings.h"
#include <drumstick/configurationdialogs.h>
#include <drumstick/settingsfactory.h>

using namespace drumstick::rt;

Connections::Connections(QWidget *parent)
    : QDialog(parent),
      m_settingsChanged(false),
      m_midiIn(nullptr),
      m_savedIn(nullptr),
      m_midiOut(nullptr),
      m_savedOut(nullptr)
{
    ui.setupUi(this);
    ui.m_advanced->setChecked(VPianoSettings::instance()->advanced());
    ui.m_thru->setChecked(VPianoSettings::instance()->midiThru());
    connect(ui.m_advanced, &QCheckBox::clicked, this, &Connections::clickedAdvanced);
    connect(ui.m_inputBackends, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Connections::refreshInputs);
    connect(ui.m_outputBackends, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Connections::refreshOutputs);
    connect(ui.btnInputDriverCfg, &QToolButton::clicked, this, &Connections::configureInputDriver);
    connect(ui.btnOutputDriverCfg, &QToolButton::clicked, this, &Connections::configureOutputDriver);
}

void Connections::setInput(MIDIInput *in)
{
    m_savedIn = m_midiIn = in;
    m_connIn = in->currentConnection();
}

void Connections::setOutput(MIDIOutput *out)
{
    m_savedOut = m_midiOut = out;
    m_connOut = out->currentConnection();
}

void Connections::setInputs(QList<MIDIInput *> ins)
{
    ui.m_inputBackends->disconnect();
    ui.m_inputBackends->clear();
    foreach(MIDIInput *i, ins) {
        ui.m_inputBackends->addItem(i->backendName(), QVariant::fromValue(i));
    }
    connect(ui.m_inputBackends, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Connections::refreshInputs);
}

void Connections::setOutputs(QList<MIDIOutput *> outs)
{
    ui.m_outputBackends->disconnect();
    foreach(MIDIOutput *o, outs) {
        ui.m_outputBackends->addItem(o->backendName(), QVariant::fromValue(o));
    }
    connect(ui.m_outputBackends, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Connections::refreshOutputs);
}

MIDIInput *Connections::getInput()
{
    return m_midiIn;
}

MIDIOutput *Connections::getOutput()
{
    return m_midiOut;
}

void Connections::reopenDrivers()
{
    drumstick::widgets::SettingsFactory settings;
    if (m_midiOut != nullptr) {
        if (m_connOut != m_midiOut->currentConnection() || m_settingsChanged) {
            m_midiOut->close();
            if (!m_connOut.first.isEmpty()) {
                m_midiOut->initialize(settings.getQSettings());
                m_midiOut->open(m_connOut);
            }
        }
    }
    if (m_midiIn != nullptr) {
        if (m_connIn != m_midiIn->currentConnection() || m_settingsChanged) {
            m_midiIn->close();
            if (!m_connIn.first.isEmpty()) {
                m_midiIn->initialize(settings.getQSettings());
                m_midiIn->open(m_connIn);
            }
        }
        if (m_midiOut != nullptr) {
            m_midiIn->setMIDIThruDevice(m_midiOut);
            m_midiIn->enableMIDIThru(VPianoSettings::instance()->midiThru());
        }
    }
    m_settingsChanged = false;
}

void Connections::accept()
{
    m_connIn = ui.m_inputPorts->currentData().value<MIDIConnection>();
    m_connOut = ui.m_outputPorts->currentData().value<MIDIConnection>();
    VPianoSettings::instance()->setAdvanced(ui.m_advanced->isChecked());
    VPianoSettings::instance()->setMidiThru(ui.m_thru->isChecked());
    reopenDrivers();
    VPianoSettings::instance()->setLastOutputBackend(m_midiOut->backendName());
    VPianoSettings::instance()->setLastOutputConnection(m_midiOut->currentConnection().first);
    VPianoSettings::instance()->setLastInputBackend(m_midiIn->backendName());
    VPianoSettings::instance()->setLastInputConnection(m_midiIn->currentConnection().first);
    QDialog::accept();
}

void Connections::reject()
{
    m_midiIn = m_savedIn;
    m_midiOut = m_savedOut;
    reopenDrivers();
    QDialog::reject();
}

void Connections::refresh()
{
    if (m_midiIn != nullptr) {
        ui.m_inputBackends->setCurrentText(m_midiIn->backendName());
        refreshInputs(ui.m_inputBackends->currentIndex());
    }
    if (m_midiOut != nullptr) {
        ui.m_outputBackends->setCurrentText(m_midiOut->backendName());
        refreshOutputs(ui.m_outputBackends->currentIndex());
    }
}

void Connections::refreshInputs(int index)
{
    QString id = ui.m_inputBackends->itemText(index);
    ui.btnInputDriverCfg->setEnabled(drumstick::widgets::inputDriverIsConfigurable(id));
    if (m_midiIn != nullptr && m_midiIn->backendName() != id) {
        m_midiIn->close();
        m_midiIn = ui.m_inputBackends->itemData(index).value<MIDIInput*>();
    }
    ui.m_inputPorts->clear();
    if (m_midiIn != nullptr) {
        ui.m_inputPorts->addItem(QString());
        const QList<MIDIConnection> conns = m_midiIn->connections(ui.m_advanced->isChecked());
        for (const MIDIConnection& conn : conns) {
            ui.m_inputPorts->addItem(conn.first, QVariant::fromValue(conn));
        }
        QString strconn = m_midiIn->currentConnection().first;
        if (strconn.isEmpty()) {
            strconn = conns.first().first;
        }
        ui.m_inputPorts->setCurrentText(strconn);
    }
}

void Connections::refreshOutputs(int index)
{
    QString id = ui.m_outputBackends->itemText(index);
    ui.btnOutputDriverCfg->setEnabled(drumstick::widgets::outputDriverIsConfigurable(id));
    if (m_midiOut != nullptr && m_midiOut->backendName() != id) {
        m_midiOut->close();
        m_midiOut = ui.m_outputBackends->itemData(index).value<MIDIOutput*>();
    }
    ui.m_outputPorts->clear();
    if (m_midiOut != nullptr) {
        const QList<MIDIConnection> conns = m_midiOut->connections(ui.m_advanced->isChecked());
        for(const MIDIConnection& conn : conns) {
            ui.m_outputPorts->addItem(conn.first, QVariant::fromValue(conn));
        }
        QString strconn = m_midiOut->currentConnection().first;
        if (strconn.isEmpty()) {
            strconn = conns.first().first;
        }
        ui.m_outputPorts->setCurrentText(strconn);
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
    if (drumstick::widgets::inputDriverIsConfigurable(driver)) {
        m_settingsChanged |= drumstick::widgets::configureInputDriver(driver, this);
    }
}

void Connections::configureOutputDriver()
{
    QString driver = ui.m_outputBackends->currentText();
    if (drumstick::widgets::outputDriverIsConfigurable(driver)) {
        m_settingsChanged |= drumstick::widgets::configureOutputDriver(driver, this);
    }
}
