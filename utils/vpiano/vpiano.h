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

#ifndef VPIANO_H
#define VPIANO_H

#include <QMainWindow>
#include <QCloseEvent>
#include <drumstick/rtmidiinput.h>
#include <drumstick/rtmidioutput.h>

#include "ui_vpiano.h"
#include "vpianoabout.h"
#include "connections.h"
#include "preferences.h"

class VPiano : public QMainWindow
{
    Q_OBJECT
public:
    VPiano( QWidget * parent = 0, Qt::WindowFlags flags = 0 );
    virtual ~VPiano();
    void closeEvent(QCloseEvent *event);
    void findInput(QString name, QList<drumstick::rt::MIDIInput*> &inputs);
    void findOutput(QString name, QList<drumstick::rt::MIDIOutput*> &outputs);

public slots:
    void readSettings();
    void writeSettings();

    void slotAbout();
    void slotAboutQt();
    void slotConnections();
    void slotPreferences();

    void slotNoteOn(const int midiNote, const int vel);
    void slotNoteOn(const int chan, const int note, const int vel);
    void slotNoteOff(const int midiNote, const int vel);
    void slotNoteOff(const int chan, const int note, const int vel);

private:
    drumstick::rt::MIDIInput * m_midiIn;
    drumstick::rt::MIDIOutput* m_midiOut;
    QString m_lastInputBackend;
    QString m_lastOutputBackend;
    QString m_lastInputConnection;
    QString m_lastOutputConnection;
    bool m_midiThru;
    bool m_advanced;
    Ui::VPiano ui;
    About dlgAbout;
    Connections dlgConnections;
    Preferences dlgPreferences;
    QString m_nativeInput;
    QString m_nativeOutput;
    QString m_defaultInput;
    QString m_defaultOutput;
};

#endif // VPIANO_H
