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

#include <QDebug>
#include "vpiano.h"
#include "backendmanager.h"

#if defined(Q_OS_LINUX)
QString nativeBackend("ALSA");
//Q_IMPORT_PLUGIN(ALSAMIDIInput)
//Q_IMPORT_PLUGIN(ALSAMIDIOutput)
#endif

#if defined(Q_OS_OSX)
QString nativeBackend("CoreMIDI");
//Q_IMPORT_PLUGIN(MacMIDIInput)
//Q_IMPORT_PLUGIN(MacMIDIOutput)
#endif

#if defined(Q_OS_WIN)
QString nativeBackend("Windows MM");
//Q_IMPORT_PLUGIN(WinMIDIInput)
//Q_IMPORT_PLUGIN(WinMIDIOutput)
#endif

//#if defined(Q_OS_UNIX)
//Q_IMPORT_PLUGIN(OSSInput)
//Q_IMPORT_PLUGIN(OSSOutput)
//#endif

//Q_IMPORT_PLUGIN(NetMIDIInput)
//Q_IMPORT_PLUGIN(NetMIDIOutput)

//#if defined(SYNTH_BACKEND)
//Q_IMPORT_PLUGIN(SynthOutput)
//#endif

VPiano::VPiano( QWidget * parent, Qt::WindowFlags flags )
    : QMainWindow(parent, flags),
    m_midiIn(0),
    m_midiOut(0)
{
    ui.setupUi(this);
    ui.statusBar->hide();
    ui.pianokeybd->setRawKeyboardMode(false);

    QSettings settings;

    BackendManager man;
    man.refresh(&settings);
    m_inputs = man.inputsAvailable();
    m_outputs = man.outputsAvailable();

    foreach(MIDIInput* input, m_inputs) {
        if (m_midiIn == 0 && (input->backendName() == nativeBackend))  {
            m_midiIn = input;
            break;
        }
    }

    foreach(MIDIOutput* output, m_outputs) {
        if (m_midiOut == 0 && (output->backendName() == nativeBackend))  {
            m_midiOut = output;
            break;
        }
    }

    connect(ui.actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(ui.actionAbout, SIGNAL(triggered()), SLOT(slotAbout()));
    connect(ui.actionAbout_Qt, SIGNAL(triggered()), SLOT(slotAboutQt()));
    connect(ui.actionConnections, SIGNAL(triggered()), SLOT(slotConnections()));
    connect(ui.actionPreferences, SIGNAL(triggered()), SLOT(slotPreferences()));
    connect(ui.pianokeybd, SIGNAL(noteOn(int)), SLOT(slotNoteOn(int)));
    connect(ui.pianokeybd, SIGNAL(noteOff(int)), SLOT(slotNoteOff(int)));

    dlgConnections.setInputs(m_inputs);
    dlgConnections.setOutputs(m_outputs);
    dlgConnections.setInput(m_midiIn);
    dlgConnections.setOutput(m_midiOut);
    if (m_midiIn != 0) {
        connect(m_midiIn, SIGNAL(midiNoteOn(int,int,int)), SLOT(slotNoteOn(int,int,int)));
        connect(m_midiIn, SIGNAL(midiNoteOff(int,int,int)), SLOT(slotNoteOff(int,int,int)));
    }

}

VPiano::~VPiano()
{
    m_midiIn->close();
    m_midiOut->close();
    qDebug() << "Cheers!";
}

void VPiano::slotNoteOn(const int midiNote)
{
    int chan = dlgPreferences.getOutChannel();
    int vel = dlgPreferences.getVelocity();
    m_midiOut->sendNoteOn(chan, midiNote, vel);
}

void VPiano::slotNoteOff(const int midiNote)
{
    int chan = dlgPreferences.getOutChannel();
    int vel = dlgPreferences.getVelocity();
    m_midiOut->sendNoteOff(chan, midiNote, vel);
}


void VPiano::slotNoteOn(const int chan, const int note, const int vel)
{
    Q_UNUSED(vel)
    if (dlgPreferences.getInChannel() == chan) {
        ui.pianokeybd->showNoteOn(note);
    }
}

void VPiano::slotNoteOff(const int chan, const int note, const int vel)
{
    Q_UNUSED(vel)
    if (dlgPreferences.getInChannel() == chan) {
        ui.pianokeybd->showNoteOff(note);
    }
}

void VPiano::slotAbout()
{
    dlgAbout.exec();
}

void VPiano::slotAboutQt()
{
    qApp->aboutQt();
}

void VPiano::slotConnections()
{
    dlgConnections.refresh();
    if (dlgConnections.exec() == QDialog::Accepted) {
        if (m_midiIn != 0) {
            m_midiIn->disconnect();
        }
        if (m_midiOut != 0) {
            m_midiOut->disconnect();
        }
        m_midiIn = dlgConnections.getInput();
        m_midiOut = dlgConnections.getOutput();
        if (m_midiIn != 0) {
            connect(m_midiIn, SIGNAL(midiNoteOn(int,int,int)), SLOT(slotNoteOn(int,int,int)));
            connect(m_midiIn, SIGNAL(midiNoteOff(int,int,int)), SLOT(slotNoteOff(int,int,int)));
        }
    }
}

void VPiano::slotPreferences()
{
    if (dlgPreferences.exec() == QDialog::Accepted) {
        if (ui.pianokeybd->baseOctave() != dlgPreferences.getBaseOctave()) {
            ui.pianokeybd->setBaseOctave(dlgPreferences.getBaseOctave());
        }
        if (ui.pianokeybd->numOctaves() != dlgPreferences.getNumOctaves()) {
            ui.pianokeybd->setNumOctaves(dlgPreferences.getNumOctaves());
        }
    }
}
