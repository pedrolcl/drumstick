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

#ifndef CONNECTIONS_H
#define CONNECTIONS_H

#include <QObject>
#include <QDialog>
#include <QShowEvent>
#include "rtmidiinput.h"
#include "rtmidioutput.h"
#include "ui_connections.h"

using namespace drumstick::rt;

class Connections : public QDialog
{
    Q_OBJECT

public:
    Connections(QWidget *parent = 0);
    void setInput(MIDIInput *in) { m_midiIn = in; }
    void setOutput(MIDIOutput *out) { m_midiOut = out; }
    void setInputs(QList<MIDIInput *> ins);
    void setOutputs(QList<MIDIOutput *> outs);
    MIDIInput *getInput() { return m_midiIn; }
    MIDIOutput *getOutput() { return m_midiOut; }
    void setAdvanced(bool value);
    bool advanced();
    bool midiThru();

public slots:
    void configureInputDriver();
    void configureOutputDriver();
    void clickedAdvanced(bool value);
    void setMidiThru(bool value);
    void refreshInputs(QString id);
    void refreshOutputs(QString id);
    void refresh();
    void accept();

private:
    bool m_advanced;
    bool m_thru;
    bool m_settingsChanged;
    MIDIInput* m_midiIn;
    MIDIOutput* m_midiOut;
    Ui::ConnectionsClass ui;
};

#endif // CONNECTIONS_H
