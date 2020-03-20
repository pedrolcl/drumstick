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

#include <QDir>
#include <QFileInfo>
#include <QDebug>
#if defined(Q_OS_MACOS)
#include <CoreFoundation/CoreFoundation.h>
#endif
#include <drumstick/backendmanager.h>
#include <drumstick/pianokeybd.h>
#include <drumstick/settingsfactory.h>
#include "vpiano.h"
#include "vpianosettings.h"
#include "vpianoabout.h"
#include "connections.h"
#include "preferences.h"

using namespace drumstick::rt;

VPiano::VPiano( QWidget * parent, Qt::WindowFlags flags )
    : QMainWindow(parent, flags),
    m_midiIn(nullptr),
    m_midiOut(nullptr)
{
    ui.setupUi(this);
    ui.statusBar->hide();
}

VPiano::~VPiano()
{
    m_midiIn->close();
    m_midiOut->close();
}

void VPiano::initialize()
{
    readSettings();

    BackendManager man;
    man.refresh(VPianoSettings::instance()->settingsMap());
    m_inputs = man.availableInputs();
    m_outputs = man.availableOutputs();

    findInput(VPianoSettings::instance()->lastInputBackend());
    if (m_midiIn == nullptr) {
        findInput(VPianoSettings::instance()->nativeInput());
        if (m_midiIn == nullptr) {
            qFatal("Unable to find a suitable input backend.");
        }
    }

    findOutput(VPianoSettings::instance()->lastOutputBackend());
    if (m_midiOut == nullptr) {
        findOutput(VPianoSettings::instance()->nativeOutput());
        if (m_midiOut == nullptr) {
            qFatal("Unable to find a suitable output backend");
        }
    }

    connect(ui.actionExit, SIGNAL(triggered()), SLOT(close()));
    connect(ui.actionAbout, SIGNAL(triggered()), SLOT(slotAbout()));
    connect(ui.actionAbout_Qt, SIGNAL(triggered()), SLOT(slotAboutQt()));
    connect(ui.actionConnections, SIGNAL(triggered()), SLOT(slotConnections()));
    connect(ui.actionPreferences, SIGNAL(triggered()), SLOT(slotPreferences()));
    connect(ui.pianokeybd, SIGNAL(noteOn(int,int)), SLOT(slotNoteOn(int,int)));
    connect(ui.pianokeybd, SIGNAL(noteOff(int,int)), SLOT(slotNoteOff(int,int)));

    drumstick::widgets::SettingsFactory settings;
    if (m_midiIn != nullptr) {
#if QT_VERSION < 0x050700
        connect(m_midiIn, SIGNAL(midiNoteOn(int,int,int)),
                          SLOT(slotNoteOn(int,int,int)),
                          Qt::QueuedConnection);
        connect(m_midiIn, SIGNAL(midiNoteOff(int,int,int)),
                          SLOT(slotNoteOff(int,int,int)),
                          Qt::QueuedConnection);
#else
        connect(m_midiIn, &MIDIInput::midiNoteOn,
                this, QOverload<int,int,int>::of(&VPiano::slotNoteOn),
                Qt::QueuedConnection);
        connect(m_midiIn, &MIDIInput::midiNoteOff,
                this, QOverload<int,int,int>::of(&VPiano::slotNoteOff),
                Qt::QueuedConnection);
#endif
        if (!VPianoSettings::instance()->lastInputConnection().isEmpty()) {
            m_midiIn->initialize(settings.getQSettings());
            auto conin = m_midiIn->connections(VPianoSettings::instance()->advanced());
            Q_ASSERT(conin.contains(VPianoSettings::instance()->lastInputConnection()));
            m_midiIn->open(VPianoSettings::instance()->lastInputConnection());
        }
    }

    if (m_midiOut != nullptr && !VPianoSettings::instance()->lastOutputConnection().isEmpty()) {
        m_midiOut->initialize(settings.getQSettings());
        auto conout = m_midiOut->connections(VPianoSettings::instance()->advanced());
        Q_ASSERT(conout.contains(VPianoSettings::instance()->lastOutputConnection()));
        m_midiOut->open(VPianoSettings::instance()->lastOutputConnection());
        if (m_midiIn != nullptr) {
            m_midiIn->setMIDIThruDevice(m_midiOut);
            m_midiIn->enableMIDIThru(VPianoSettings::instance()->midiThru());
        }
    }
}

void VPiano::showEvent(QShowEvent *event)
{
    initialize();
    event->accept();
}

void VPiano::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void VPiano::slotNoteOn(const int midiNote, const int vel)
{
    int chan = VPianoSettings::instance()->outChannel();
    m_midiOut->sendNoteOn(chan, midiNote, vel);
}

void VPiano::slotNoteOff(const int midiNote, const int vel)
{
    int chan = VPianoSettings::instance()->outChannel();
    m_midiOut->sendNoteOff(chan, midiNote, vel);
}

void VPiano::slotNoteOn(const int chan, const int note, const int vel)
{
    if (VPianoSettings::instance()->inChannel() == chan) {
        if (vel > 0)
            ui.pianokeybd->showNoteOn(note);
        else
            ui.pianokeybd->showNoteOff(note);
    }
}

void VPiano::slotNoteOff(const int chan, const int note, const int vel)
{
    Q_UNUSED(vel)
    if (VPianoSettings::instance()->inChannel() == chan) {
        ui.pianokeybd->showNoteOff(note);
    }
}

void VPiano::slotAbout()
{
    About dlgAbout(this);
    dlgAbout.exec();
}

void VPiano::slotAboutQt()
{
    qApp->aboutQt();
}

void VPiano::slotConnections()
{
    Connections dlgConnections(this);
    dlgConnections.setInputs(m_inputs);
    dlgConnections.setOutputs(m_outputs);
    dlgConnections.setInput(m_midiIn);
    dlgConnections.setOutput(m_midiOut);
    dlgConnections.refresh();
    if (dlgConnections.exec() == QDialog::Accepted) {
        if (m_midiIn != nullptr) {
            m_midiIn->disconnect();
        }
        if (m_midiOut != nullptr) {
            m_midiOut->disconnect();
        }
        m_midiIn = dlgConnections.getInput();
        m_midiOut = dlgConnections.getOutput();
        if (m_midiIn != nullptr) {
            connect(m_midiIn, SIGNAL(midiNoteOn(int,int,int)), SLOT(slotNoteOn(int,int,int)));
            connect(m_midiIn, SIGNAL(midiNoteOff(int,int,int)), SLOT(slotNoteOff(int,int,int)));
        }
    }
}

void VPiano::slotPreferences()
{
    Preferences dlgPreferences(this);
    if (dlgPreferences.exec() == QDialog::Accepted) {
        if (ui.pianokeybd->baseOctave() != VPianoSettings::instance()->baseOctave()) {
            ui.pianokeybd->setBaseOctave(VPianoSettings::instance()->baseOctave());
        }
        if (ui.pianokeybd->numKeys() != VPianoSettings::instance()->numKeys()) {
            ui.pianokeybd->setNumKeys(VPianoSettings::instance()->numKeys());
        }
        if (ui.pianokeybd->startKey() != VPianoSettings::instance()->startingKey()) {
            ui.pianokeybd->setNumKeys(VPianoSettings::instance()->numKeys(), VPianoSettings::instance()->startingKey());
        }
    }
}

void VPiano::writeSettings()
{
    VPianoSettings::instance()->setGeometry(saveGeometry());
    VPianoSettings::instance()->setState(saveState());
    VPianoSettings::instance()->SaveSettings();
}

void VPiano::readSettings()
{
    VPianoSettings::instance()->ReadSettings();
    restoreGeometry(VPianoSettings::instance()->geometry());
    restoreState(VPianoSettings::instance()->state());
    ui.pianokeybd->setBaseOctave(VPianoSettings::instance()->baseOctave());
    ui.pianokeybd->setNumKeys(VPianoSettings::instance()->numKeys(), VPianoSettings::instance()->startingKey());
}

void VPiano::findInput(QString name)
{
    if (name.isEmpty()) {
        return;
    }
    foreach(MIDIInput* input, m_inputs) {
        if (m_midiIn == nullptr && (input->backendName() == name))  {
            m_midiIn = input;
            break;
        }
    }
    if (m_midiIn == nullptr) {
        qWarning() << "Input backend not found: " << name;
    }
}

void VPiano::findOutput(QString name)
{
    if (name.isEmpty()) {
        return;
    }
    foreach(MIDIOutput* output, m_outputs) {
        if (m_midiOut == nullptr && (output->backendName() == name))  {
            m_midiOut = output;
            break;
        }
    }
    if (m_midiOut == nullptr) {
        qWarning() << "Output backend not found: " << name;
    }
}

void VPiano::setPortableConfig(const QString fileName)
{
    if (fileName.isEmpty()) {
        QFileInfo appInfo(QCoreApplication::applicationFilePath());
#if defined(Q_OS_MACOS)
        CFURLRef url = static_cast<CFURLRef>(CFAutorelease(static_cast<CFURLRef>(CFBundleCopyBundleURL(CFBundleGetMainBundle()))));
        QString path = QUrl::fromCFURL(url).path() + "../";
        QFileInfo cfgInfo(path, appInfo.baseName() + ".conf");
#else
        QFileInfo cfgInfo(appInfo.absoluteDir(), appInfo.baseName() + ".conf");
#endif
        drumstick::widgets::SettingsFactory::setFileName(cfgInfo.absoluteFilePath());
    } else {
        drumstick::widgets::SettingsFactory::setFileName(fileName);
    }
}
