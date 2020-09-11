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
#include <QFontDialog>
#include <QInputDialog>
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
using namespace drumstick::widgets;

VPiano::VPiano( QWidget * parent, Qt::WindowFlags flags )
    : QMainWindow(parent, flags),
    m_midiIn(nullptr),
    m_midiOut(nullptr)
{
    ui.setupUi(this);

    bool mouseInputEnabled = true;
    bool touchInputEnabled = false;
    for(const QTouchDevice *dev : QTouchDevice::devices()) {
        if (dev->type() == QTouchDevice::TouchScreen) {
            mouseInputEnabled = false;
            touchInputEnabled = true;
            break;
        }
    }
    ui.pianokeybd->setMouseEnabled(mouseInputEnabled);
    ui.pianokeybd->setTouchEnabled(touchInputEnabled);

    connect(ui.pianokeybd, &PianoKeybd::noteOn, this, QOverload<int,int>::of(&VPiano::slotNoteOn));
    connect(ui.pianokeybd, &PianoKeybd::noteOff, this, QOverload<int,int>::of(&VPiano::slotNoteOff));
    connect(ui.pianokeybd, &PianoKeybd::signalName, this, &VPiano::slotNoteName);
    connect(ui.actionExit, &QAction::triggered, this,  &VPiano::close);
    connect(ui.actionAbout, &QAction::triggered, this,  &VPiano::slotAbout);
    connect(ui.actionAbout_Qt, &QAction::triggered, this,  &VPiano::slotAboutQt);
    connect(ui.actionConnections, &QAction::triggered, this,  &VPiano::slotConnections);
    connect(ui.actionPreferences, &QAction::triggered, this,  &VPiano::slotPreferences);
    connect(ui.actionNames_Font, &QAction::triggered, this, &VPiano::slotChangeFont);

    QActionGroup* nameGroup = new QActionGroup(this);
    nameGroup->setExclusive(true);
    nameGroup->addAction(ui.actionStandard);
    nameGroup->addAction(ui.actionCustom_Sharps);
    nameGroup->addAction(ui.actionCustom_Flats);
    connect(ui.actionStandard,      &QAction::triggered, this, &VPiano::slotStandardNames);
    connect(ui.actionCustom_Sharps, &QAction::triggered, [=]{ slotCustomNames(true); });
    connect(ui.actionCustom_Flats,  &QAction::triggered, [=]{ slotCustomNames(false); });

    QActionGroup* nameVisibilityGroup = new QActionGroup(this);
    nameVisibilityGroup->setExclusive(true);
    nameVisibilityGroup->addAction(ui.actionNever);
    nameVisibilityGroup->addAction(ui.actionMinimal);
    nameVisibilityGroup->addAction(ui.actionWhen_Activated);
    nameVisibilityGroup->addAction(ui.actionAlways);
    connect(nameVisibilityGroup, &QActionGroup::triggered, this, &VPiano::slotNameVisibility);

    QActionGroup* blackKeysGroup = new QActionGroup(this);
    blackKeysGroup->setExclusive(true);
    blackKeysGroup->addAction(ui.actionFlats);
    blackKeysGroup->addAction(ui.actionSharps);
    blackKeysGroup->addAction(ui.actionNothing);
    connect(blackKeysGroup, &QActionGroup::triggered, this, &VPiano::slotNameVariant);

    QActionGroup* orientationGroup = new QActionGroup(this);
    orientationGroup->setExclusive(true);
    orientationGroup->addAction(ui.actionHorizontal);
    orientationGroup->addAction(ui.actionVertical);
    orientationGroup->addAction(ui.actionAutomatic);
    connect(orientationGroup, &QActionGroup::triggered, this, &VPiano::slotNameOrientation);

    QActionGroup* centralOctaveGroup = new QActionGroup(this);
    centralOctaveGroup->setExclusive(true);
    centralOctaveGroup->addAction(ui.actionC3);
    centralOctaveGroup->addAction(ui.actionC4);
    centralOctaveGroup->addAction(ui.actionC5);
    connect(centralOctaveGroup, &QActionGroup::triggered, this, &VPiano::slotCentralOctave);

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

    drumstick::widgets::SettingsFactory settings;
    if (m_midiIn != nullptr) {

        connect(m_midiIn, &MIDIInput::midiNoteOn,
                this, QOverload<int,int,int>::of(&VPiano::slotNoteOn),
                Qt::QueuedConnection);
        connect(m_midiIn, &MIDIInput::midiNoteOff,
                this, QOverload<int,int,int>::of(&VPiano::slotNoteOff),
                Qt::QueuedConnection);

        if (!VPianoSettings::instance()->lastInputConnection().isEmpty()) {
            m_midiIn->initialize(settings.getQSettings());
            auto conin = m_midiIn->connections(VPianoSettings::instance()->advanced());
            auto lastIn = VPianoSettings::instance()->lastInputConnection();
            for(const MIDIConnection& conn: conin) {
                if (conn.first == lastIn) {
                    m_midiIn->open(conn);
                    break;
                }
            }
        }
    }

    auto lastConnOut = VPianoSettings::instance()->lastOutputConnection();
    if (m_midiOut != nullptr && !lastConnOut.isEmpty()) {
        m_midiOut->initialize(settings.getQSettings());
        auto connOut = m_midiOut->connections(VPianoSettings::instance()->advanced());
        for(const MIDIConnection& conn : connOut) {
            if (conn.first == lastConnOut) {
                m_midiOut->open(conn);
                if (m_midiIn != nullptr) {
                    m_midiIn->setMIDIThruDevice(m_midiOut);
                    m_midiIn->enableMIDIThru(VPianoSettings::instance()->midiThru());
                }
                break;
            }
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
        if (vel > 0) {
            ui.pianokeybd->showNoteOn(note, vel);
        } else {
            ui.pianokeybd->showNoteOff(note);
        }
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
            connect(m_midiIn, &MIDIInput::midiNoteOn, this,
                    QOverload<int,int,int>::of(&VPiano::slotNoteOn), Qt::QueuedConnection);
            connect(m_midiIn, &MIDIInput::midiNoteOff, this,
                    QOverload<int,int,int>::of(&VPiano::slotNoteOff), Qt::QueuedConnection);
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
        if (ui.pianokeybd->numKeys() != VPianoSettings::instance()->numKeys() ||
            ui.pianokeybd->startKey() != VPianoSettings::instance()->startingKey()) {
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

    ui.pianokeybd->setFont(VPianoSettings::instance()->namesFont());

    PianoKeybd::LabelNaming namingPolicy = VPianoSettings::instance()->namingPolicy();
    switch(namingPolicy) {
    case PianoKeybd::StandardNames:
        ui.actionStandard->setChecked(true);
        ui.pianokeybd->useStandardNoteNames();
        break;
    case PianoKeybd::CustomNamesWithSharps:
        ui.actionCustom_Sharps->setChecked(true);
        ui.pianokeybd->useCustomNoteNames(VPianoSettings::instance()->names_sharps());
        break;
    case PianoKeybd::CustomNamesWithFlats:
        ui.actionCustom_Flats->setChecked(true);
        ui.pianokeybd->useCustomNoteNames(VPianoSettings::instance()->names_flats());
        break;
    }

    PianoKeybd::LabelOrientation nOrientation = VPianoSettings::instance()->namesOrientation();
    ui.pianokeybd->setLabelOrientation(nOrientation);
    switch(nOrientation) {
    case PianoKeybd::HorizontalOrientation:
        ui.actionHorizontal->setChecked(true);
        break;
    case PianoKeybd::VerticalOrientation:
        ui.actionVertical->setChecked(true);
        break;
    case PianoKeybd::AutomaticOrientation:
        ui.actionAutomatic->setChecked(true);
        break;
    default:
        break;
    }

    PianoKeybd::LabelAlteration alteration = VPianoSettings::instance()->alterations();
    ui.pianokeybd->setLabelAlterations(alteration);
    switch(alteration) {
    case PianoKeybd::ShowSharps:
        ui.actionSharps->setChecked(true);
        break;
    case PianoKeybd::ShowFlats:
        ui.actionFlats->setChecked(true);
        break;
    case PianoKeybd::ShowNothing:
        ui.actionNothing->setChecked(true);
        break;
    default:
        break;
    }

    PianoKeybd::LabelVisibility visibility = VPianoSettings::instance()->namesVisibility();
    ui.pianokeybd->setShowLabels(visibility);
    switch(visibility) {
    case PianoKeybd::ShowNever:
        ui.actionNever->setChecked(true);
        break;
    case PianoKeybd::ShowMinimum:
        ui.actionMinimal->setChecked(true);
        break;
    case PianoKeybd::ShowActivated:
        ui.actionWhen_Activated->setChecked(true);
        break;
    case PianoKeybd::ShowAlways:
        ui.actionAlways->setChecked(true);
        break;
    default:
        break;
    }

    PianoKeybd::LabelCentralOctave nOctave = VPianoSettings::instance()->namesOctave();
    ui.pianokeybd->setLabelOctave(nOctave);
    switch(nOctave) {
    case PianoKeybd::OctaveC3:
        ui.actionC3->setChecked(true);
        break;
    case PianoKeybd::OctaveC4:
        ui.actionC4->setChecked(true);
        break;
    case PianoKeybd::OctaveC5:
        ui.actionC5->setChecked(true);
        break;
    default:
        break;
    }

    ui.statusBar->show();

    ui.pianokeybd->setBaseOctave(VPianoSettings::instance()->baseOctave());
    ui.pianokeybd->setNumKeys(VPianoSettings::instance()->numKeys(), VPianoSettings::instance()->startingKey());

    PianoPalette palette(1, PAL_SINGLE);
    palette.setColor(0, QString(), Qt::red);
    ui.pianokeybd->setPianoPalette(palette);
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

void VPiano::useCustomNoteNames()
{
    if (ui.pianokeybd->labelAlterations() == PianoKeybd::ShowFlats) {
        ui.pianokeybd->useCustomNoteNames(VPianoSettings::instance()->names_flats());
    } else {
        ui.pianokeybd->useCustomNoteNames(VPianoSettings::instance()->names_sharps());
    }
}

void VPiano::slotChangeFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok,
                    VPianoSettings::instance()->namesFont(),
                    this, tr("Font to display note names"),
                    QFontDialog::DontUseNativeDialog | QFontDialog::ScalableFonts);
    if (ok) {
        VPianoSettings::instance()->setNamesFont(font);
        ui.pianokeybd->setFont(font);
    }
}

void VPiano::slotNameOrientation(QAction* action)
{
    if(action == ui.actionHorizontal) {
        VPianoSettings::instance()->setNamesOrientation(PianoKeybd::HorizontalOrientation);
    } else if(action == ui.actionVertical) {
        VPianoSettings::instance()->setNamesOrientation(PianoKeybd::VerticalOrientation);
    } else if(action == ui.actionAutomatic) {
        VPianoSettings::instance()->setNamesOrientation(PianoKeybd::AutomaticOrientation);
    }
    ui.pianokeybd->setLabelOrientation(VPianoSettings::instance()->namesOrientation());
}

void VPiano::slotNameVisibility(QAction* action)
{
    if(action == ui.actionNever) {
        VPianoSettings::instance()->setNamesVisibility(PianoKeybd::ShowNever);
    } else if(action == ui.actionMinimal) {
        VPianoSettings::instance()->setNamesVisibility(PianoKeybd::ShowMinimum);
    } else if(action == ui.actionWhen_Activated) {
        VPianoSettings::instance()->setNamesVisibility(PianoKeybd::ShowActivated);
    } else if(action == ui.actionAlways) {
        VPianoSettings::instance()->setNamesVisibility(PianoKeybd::ShowAlways);
    }
    ui.pianokeybd->setShowLabels(VPianoSettings::instance()->namesVisibility());
}

void VPiano::slotNameVariant(QAction* action)
{
    if(action == ui.actionSharps) {
        VPianoSettings::instance()->setNamesAlterations(PianoKeybd::ShowSharps);
    } else if(action == ui.actionFlats) {
        VPianoSettings::instance()->setNamesAlterations(PianoKeybd::ShowFlats);
    } else if(action == ui.actionNothing) {
        VPianoSettings::instance()->setNamesAlterations(PianoKeybd::ShowNothing);
    }
    ui.pianokeybd->setLabelAlterations(VPianoSettings::instance()->alterations());
}

void VPiano::slotCentralOctave(QAction *action)
{
    if (action == ui.actionC3) {
        VPianoSettings::instance()->setNamesOctave(PianoKeybd::OctaveC3);
    } else if(action == ui.actionC4) {
        VPianoSettings::instance()->setNamesOctave(PianoKeybd::OctaveC4);
    } else if(action == ui.actionC5) {
        VPianoSettings::instance()->setNamesOctave(PianoKeybd::OctaveC5);
    }
    ui.pianokeybd->setLabelOctave(VPianoSettings::instance()->namesOctave());
}

void VPiano::slotStandardNames()
{
    VPianoSettings::instance()->setNamingPolicy(PianoKeybd::StandardNames);
    ui.pianokeybd->useStandardNoteNames();
}

void VPiano::slotCustomNames(bool sharps)
{
    bool ok;
    QString names;
    if ( sharps ) {
        names = VPianoSettings::instance()->names_sharps().join('\n');
    } else {
        names = VPianoSettings::instance()->names_flats().join('\n');
    }
    QString text = QInputDialog::getMultiLineText(this,tr("Custom Note Names"),tr("Names:"),
                                                  names, &ok);
    if (ok && !text.isEmpty()) {
        QStringList customNames = text.split('\n');
        if (sharps) {
            VPianoSettings::instance()->setNames_sharps(customNames);
            VPianoSettings::instance()->setNamingPolicy(PianoKeybd::CustomNamesWithSharps);
        } else {
            VPianoSettings::instance()->setNames_flats(customNames);
            VPianoSettings::instance()->setNamingPolicy(PianoKeybd::CustomNamesWithFlats);
        }
        ui.pianokeybd->useCustomNoteNames(customNames);
    } else {
        slotStandardNames();
    }
}

void VPiano::slotNoteName(const QString& name)
{
    if (name.isEmpty()) {
        ui.statusBar->clearMessage();
    } else {
        ui.statusBar->showMessage(name);
    }
}
