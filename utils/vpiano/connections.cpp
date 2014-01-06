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

#include "connections.h"

Connections::Connections(QWidget *parent)
    : QDialog(parent),
      m_midiIn(0),
      m_midiOut(0)
{
    ui.setupUi(this);
    connect(ui.m_advanced, SIGNAL(toggled(bool)), SLOT(refresh()));
}

void Connections::accept()
{
    QString conn = ui.m_inputs->currentText();
    if (m_midiIn != 0) {
        if (conn != m_midiIn->currentConnection()) {
            m_midiIn->close();
            if (!conn.isEmpty())
                m_midiIn->open(conn);
        }
        m_midiIn->enableMIDIThru(ui.m_thru->isChecked());
    }
    if (m_midiOut != 0) {
        conn = ui.m_outputs->currentText();
        if (conn != m_midiOut->currentConnection()) {
            m_midiOut->close();
            if (!conn.isEmpty())
                m_midiOut->open(conn);
        }
    }
    QDialog::accept();
}

void Connections::refresh()
{
    bool advanced = ui.m_advanced->isChecked();
    ui.m_inputs->clear();
    ui.m_outputs->clear();
    if (m_midiIn != 0) {
        ui.m_inputs->addItem(QString());
        ui.m_inputs->addItems(m_midiIn->connections(advanced));
        ui.m_inputs->setCurrentText(m_midiIn->currentConnection());
    }
    if (m_midiOut != 0) {
        ui.m_outputs->addItems(m_midiOut->connections(advanced));
        ui.m_outputs->setCurrentText(m_midiOut->currentConnection());
    }
}
