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

#ifndef CONNECTIONS_H
#define CONNECTIONS_H

#include <QObject>
#include <QDialog>
#include <QShowEvent>
#include <drumstick/rtmidiinput.h>
#include <drumstick/rtmidioutput.h>
#include "ui_connections.h"

class Connections : public QDialog
{
    Q_OBJECT

public:
    Connections(QWidget *parent = nullptr);
    void setInput(drumstick::rt::MIDIInput *in);
    void setOutput(drumstick::rt::MIDIOutput *out);
    void setInputs(QList<drumstick::rt::MIDIInput *> ins);
    void setOutputs(QList<drumstick::rt::MIDIOutput *> outs);
    drumstick::rt::MIDIInput *getInput();
    drumstick::rt::MIDIOutput *getOutput();

public slots:
    void configureInputDriver();
    void configureOutputDriver();
    void clickedAdvanced(bool value);
    void refreshInputs(QString id);
    void refreshOutputs(QString id);
    void refresh();
    void accept() override;

private:
    bool m_settingsChanged;
    drumstick::rt::MIDIInput* m_midiIn;
    drumstick::rt::MIDIOutput* m_midiOut;
    Ui::ConnectionsClass ui;
};

#endif // CONNECTIONS_H
