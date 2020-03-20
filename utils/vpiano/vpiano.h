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

class VPiano : public QMainWindow
{
    Q_OBJECT
public:
    VPiano( QWidget * parent = 0, Qt::WindowFlags flags = 0 );
    virtual ~VPiano();
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

    void findInput(QString name);
    void findOutput(QString name);
    void setPortableConfig(const QString fileName = QString());

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
    void initialize();

    QList<drumstick::rt::MIDIInput*> m_inputs;
    QList<drumstick::rt::MIDIOutput*> m_outputs;
    drumstick::rt::MIDIInput * m_midiIn;
    drumstick::rt::MIDIOutput* m_midiOut;
    Ui::VPiano ui;
};

#endif // VPIANO_H
