/*
    Virtual Piano test using the MIDI Sequencer C++ library
    Copyright (C) 2006-2025, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#ifndef VPIANO_H
#define VPIANO_H

#include <QCloseEvent>
#include <QMainWindow>

#include <drumstick/backendmanager.h>
#include <drumstick/rtmidiinput.h>
#include <drumstick/rtmidioutput.h>

#include "ui_vpiano.h"

class VPiano : public QMainWindow
{
    Q_OBJECT
public:
    VPiano( QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::Window);
    virtual ~VPiano();
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void setPortableConfig(const QString fileName = QString());

public Q_SLOTS:
    void readSettings();
    void writeSettings();

    void slotAbout();
    void slotConnections();
    void slotPreferences();

    void slotNoteOn(const int midiNote, const int vel);
    void slotNoteOn(const int chan, const int note, const int vel);
    void slotNoteOff(const int midiNote, const int vel);
    void slotNoteOff(const int chan, const int note, const int vel);

    void slotChangeFont();
    void slotNameOrientation(QAction* action);
    void slotNameVisibility(QAction* action);
    void slotNameVariant(QAction* action);
    void slotCentralOctave(QAction* action);
    void slotStandardNames();
    void slotCustomNames(bool sharps);
    void slotNoteName(const QString& name);
    void slotInvertedColors(bool checked);
    void slotRawKeyboard(bool checked);
    void slotKeyboardInput(bool checked);
    void slotMouseInput(bool checked);
    void slotTouchScreenInput(bool checked);
    void slotOctaveSubscript(bool checked);

private:
    void initialize();
    void useCustomNoteNames();

    drumstick::rt::BackendManager *m_manager;
    QList<drumstick::rt::MIDIInput*> m_inputs;
    QList<drumstick::rt::MIDIOutput*> m_outputs;
    drumstick::rt::MIDIInput * m_midiIn;
    drumstick::rt::MIDIOutput* m_midiOut;
    Ui::VPiano ui;

//  QStringList m_names_s{"do", "do♯", "re", "re♯", "mi", "fa", "fa♯", "sol", "sol♯", "la", "la♯", "si"};
//  QStringList m_names_f{"do", "re♭", "re", "mi♭", "mi", "fa", "sol♭", "sol", "la♭", "la", "si♭", "si"};
};

#endif // VPIANO_H
