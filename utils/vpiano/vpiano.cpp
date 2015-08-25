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

#include <QDebug>
#include "vpiano.h"
#include "backendmanager.h"

VPiano::VPiano( QWidget * parent, Qt::WindowFlags flags )
    : QMainWindow(parent, flags),
    m_midiIn(0),
    m_midiOut(0)
{
    QString nativeBackend;
#if defined(Q_OS_LINUX)
    nativeBackend = QLatin1Literal("ALSA");
#elif defined(Q_OS_OSX)
    nativeBackend = QLatin1Literal("CoreMIDI");
#elif defined(Q_OS_WIN)
    nativeBackend = QLatin1Literal("Windows MM");
#else
    nativeBackend = QLatin1Literal("Network");
#endif

    ui.setupUi(this);
    ui.statusBar->hide();

    readSettings();

    QSettings settings;
    BackendManager man;
    man.refresh(&settings);
    QList<MIDIInput*> inputs = man.availableInputs();
    QList<MIDIOutput*> outputs = man.availableOutputs();

    findInput(m_lastInputBackend, inputs);
    if (m_midiIn == 0) {
        findInput(nativeBackend, inputs);
    }

    findOutput(m_lastOutputBackend, outputs);
    if (m_midiOut == 0) {
        findOutput(nativeBackend, outputs);
    }

    connect(ui.actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(ui.actionAbout, SIGNAL(triggered()), SLOT(slotAbout()));
    connect(ui.actionAbout_Qt, SIGNAL(triggered()), SLOT(slotAboutQt()));
    connect(ui.actionConnections, SIGNAL(triggered()), SLOT(slotConnections()));
    connect(ui.actionPreferences, SIGNAL(triggered()), SLOT(slotPreferences()));
    connect(ui.pianokeybd, SIGNAL(noteOn(int,int)), SLOT(slotNoteOn(int,int)));
    connect(ui.pianokeybd, SIGNAL(noteOff(int,int)), SLOT(slotNoteOff(int,int)));

    dlgConnections.setInputs(inputs);
    dlgConnections.setOutputs(outputs);
    dlgConnections.setInput(m_midiIn);
    dlgConnections.setOutput(m_midiOut);
    dlgConnections.setMidiThru(m_midiThru);
    dlgConnections.setAdvanced(m_advanced);

    if (m_midiIn != 0) {
        connect(m_midiIn, SIGNAL(midiNoteOn(int,int,int)), SLOT(slotNoteOn(int,int,int)));
        connect(m_midiIn, SIGNAL(midiNoteOff(int,int,int)), SLOT(slotNoteOff(int,int,int)));
        if (!m_lastInputConnection.isEmpty()) {
            m_midiIn->initialize(&settings);
            m_midiIn->open(m_lastInputConnection);
        }
    }

    if (m_midiOut != 0 && !m_lastOutputConnection.isEmpty()) {
        m_midiOut->initialize(&settings);
        m_midiOut->open(m_lastOutputConnection);
        m_midiIn->setMIDIThruDevice(m_midiOut);
        m_midiIn->enableMIDIThru(m_midiThru);
    }
}

VPiano::~VPiano()
{
    m_midiIn->close();
    m_midiOut->close();
    qDebug() << "Cheers!";
}

void VPiano::slotNoteOn(const int midiNote, const int vel)
{
    int chan = dlgPreferences.getOutChannel();
    m_midiOut->sendNoteOn(chan, midiNote, vel);
}

void VPiano::slotNoteOff(const int midiNote, const int vel)
{
    int chan = dlgPreferences.getOutChannel();
    m_midiOut->sendNoteOff(chan, midiNote, vel);
}

void VPiano::slotNoteOn(const int chan, const int note, const int vel)
{
    //Q_UNUSED(vel)
    if (dlgPreferences.getInChannel() == chan) {
        if (vel > 0)
            ui.pianokeybd->getPianoScene()->showNoteOn(note);
        else
            ui.pianokeybd->getPianoScene()->showNoteOff(note);
    }
}

void VPiano::slotNoteOff(const int chan, const int note, const int vel)
{
    Q_UNUSED(vel)
    if (dlgPreferences.getInChannel() == chan) {
        ui.pianokeybd->getPianoScene()->showNoteOff(note);
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
        m_midiThru = dlgConnections.midiThru();
        m_advanced = dlgConnections.advanced();
    }
}

void VPiano::slotPreferences()
{
    if (dlgPreferences.exec() == QDialog::Accepted) {
        if (ui.pianokeybd->baseOctave() != dlgPreferences.getBaseOctave()) {
            ui.pianokeybd->setBaseOctave(dlgPreferences.getBaseOctave());
        }
        if (ui.pianokeybd->numKeys() != dlgPreferences.getNumKeys()) {
            ui.pianokeybd->setNumKeys(dlgPreferences.getNumKeys());
        }
        if (ui.pianokeybd->startKey() != dlgPreferences.getStartingKey()) {
            ui.pianokeybd->setNumKeys(dlgPreferences.getNumKeys(), dlgPreferences.getStartingKey());
        }
    }
}

void VPiano::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void VPiano::writeSettings()
{
    QSettings settings;

    settings.beginGroup("Window");
    settings.setValue("Geometry", saveGeometry());
    settings.setValue("State", saveState());
    settings.endGroup();

    settings.beginGroup("Connections");
    settings.setValue("inputBackend", m_midiIn->backendName());
    settings.setValue("outputBackend", m_midiOut->backendName());
    settings.setValue("inputConenction", m_midiIn->currentConnection());
    settings.setValue("outputConnection", m_midiOut->currentConnection());
    settings.setValue("midiThru", m_midiThru);
    settings.setValue("advanced", m_advanced);
    settings.endGroup();

    settings.beginGroup("Preferences");
    settings.setValue("inputChannel", dlgPreferences.getInChannel());
    settings.setValue("outputChannel",dlgPreferences.getOutChannel());
    settings.setValue("velocity", dlgPreferences. getVelocity());
    settings.setValue("baseOctave", dlgPreferences.getBaseOctave());
    settings.setValue("numKeys", dlgPreferences.getNumKeys());
    settings.setValue("startingKey", dlgPreferences.getStartingKey());
    settings.endGroup();

    settings.sync();
}

void VPiano::readSettings()
{
    QSettings settings;

    settings.beginGroup("Window");
    restoreGeometry(settings.value("Geometry").toByteArray());
    restoreState(settings.value("State").toByteArray());
    settings.endGroup();

    settings.beginGroup("Connections");
    m_lastInputBackend = settings.value("inputBackend").toString();
    m_lastOutputBackend = settings.value("outputBackend").toString();
    m_lastInputConnection = settings.value("inputConenction").toString();
    m_lastOutputConnection = settings.value("outputConnection").toString();
    m_midiThru = settings.value("midiThru", false).toBool();
    m_advanced = settings.value("advanced", false).toBool();
    settings.endGroup();

    settings.beginGroup("Preferences");
    dlgPreferences.setInChannel(settings.value("inputChannel", 0).toInt());
    dlgPreferences.setOutChannel(settings.value("outputChannel", 0).toInt());
    dlgPreferences.setVelocity(settings.value("velocity", 100).toInt());
    dlgPreferences.setBaseOctave(settings.value("baseOctave", 1).toInt());
    dlgPreferences.setNumKeys(settings.value("numKeys", 88).toInt());
    dlgPreferences.setStartingKey(settings.value("startingKey", 9).toInt());
    ui.pianokeybd->setBaseOctave(settings.value("baseOctave", 1).toInt());
    ui.pianokeybd->setNumKeys(settings.value("numKeys", 88).toInt(), settings.value("startingKey", 9).toInt());
    settings.endGroup();
}

void VPiano::findInput(QString name, QList<MIDIInput *> &inputs)
{
    foreach(MIDIInput* input, inputs) {
        if (m_midiIn == 0 && (input->backendName() == name))  {
            m_midiIn = input;
            break;
        }
    }
}

void VPiano::findOutput(QString name, QList<MIDIOutput *> &outputs)
{
    foreach(MIDIOutput* output, outputs) {
        if (m_midiOut == 0 && (output->backendName() == name))  {
            m_midiOut = output;
            break;
        }
    }
}
