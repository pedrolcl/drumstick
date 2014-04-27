/*
    Virtual Piano test using the MIDI Sequencer C++ library 
    Copyright (C) 2006-2014, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

public slots:
    void refreshInputs(QString id);
    void refreshOutputs(QString id);
    void refresh();
    void accept();

private:
    Ui::ConnectionsClass ui;
    MIDIInput* m_midiIn;
    MIDIOutput* m_midiOut;
    QHash<QString, MIDIInput*> m_inputs;
    QHash<QString, MIDIOutput*> m_outputs;
};

#endif // CONNECTIONS_H
