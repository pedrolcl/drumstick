/*
    Virtual Piano test using the MIDI Sequencer C++ library
    Copyright (C) 2006-2024, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QFontDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QActionGroup>
#if defined(Q_OS_MACOS)
#include <CoreFoundation/CoreFoundation.h>
#endif
#include "connections.h"
#include "preferences.h"
#include "vpiano.h"
#include "vpianoabout.h"
#include "vpianosettings.h"
#include <drumstick/backendmanager.h>
#include <drumstick/pianokeybd.h>
#include <drumstick/settingsfactory.h>

using namespace drumstick::rt;
using namespace drumstick::widgets;

VPiano::VPiano( QWidget * parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags),
    m_midiIn(nullptr),
    m_midiOut(nullptr)
{
    ui.setupUi(this);

    connect(ui.pianokeybd, &PianoKeybd::noteOn, this, QOverload<int,int>::of(&VPiano::slotNoteOn));
    connect(ui.pianokeybd, &PianoKeybd::noteOff, this, QOverload<int,int>::of(&VPiano::slotNoteOff));
    connect(ui.pianokeybd, &PianoKeybd::signalName, this, &VPiano::slotNoteName);
    connect(ui.actionExit, &QAction::triggered, this,  &VPiano::close);
    connect(ui.actionAbout, &QAction::triggered, this,  &VPiano::slotAbout);
    connect(ui.actionAbout_Qt, &QAction::triggered, qApp, &QApplication::aboutQt);
    connect(ui.actionConnections, &QAction::triggered, this,  &VPiano::slotConnections);
    connect(ui.actionPreferences, &QAction::triggered, this,  &VPiano::slotPreferences);
    connect(ui.actionNames_Font, &QAction::triggered, this, &VPiano::slotChangeFont);
    connect(ui.actionInverted_Keys_Color, &QAction::triggered, this, &VPiano::slotInvertedColors);
    connect(ui.actionRaw_Computer_Keyboard, &QAction::triggered, this, &VPiano::slotRawKeyboard);
    connect(ui.actionComputer_Keyboard_Input, &QAction::triggered, this, &VPiano::slotKeyboardInput);
    connect(ui.actionMouse_Input, &QAction::triggered, this, &VPiano::slotMouseInput);
    connect(ui.actionTouch_Screen_Input, &QAction::triggered, this, &VPiano::slotTouchScreenInput);
    connect(ui.actionOctave_Subscript_Designation, &QAction::triggered, this, &VPiano::slotOctaveSubscript);

    QActionGroup* nameGroup = new QActionGroup(this);
    nameGroup->setExclusive(true);
    nameGroup->addAction(ui.actionStandard);
    nameGroup->addAction(ui.actionCustom_Sharps);
    nameGroup->addAction(ui.actionCustom_Flats);
    connect(ui.actionStandard,      &QAction::triggered, this, &VPiano::slotStandardNames);
    connect(ui.actionCustom_Sharps, &QAction::triggered, this, [=]{ slotCustomNames(true); });
    connect(ui.actionCustom_Flats,  &QAction::triggered, this, [=]{ slotCustomNames(false); });

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
    centralOctaveGroup->addAction(ui.actionNoOctaves);
    centralOctaveGroup->addAction(ui.actionC3);
    centralOctaveGroup->addAction(ui.actionC4);
    centralOctaveGroup->addAction(ui.actionC5);
    connect(centralOctaveGroup, &QActionGroup::triggered, this, &VPiano::slotCentralOctave);

    ui.statusBar->hide();
}

VPiano::~VPiano()
{
    qDebug() << Q_FUNC_INFO;
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

    m_midiIn = man.findInput(VPianoSettings::instance()->lastInputBackend());
    if (m_midiIn == nullptr) {
        qFatal("Unable to find a suitable input backend.");
    }

    m_midiOut = man.findOutput(VPianoSettings::instance()->lastOutputBackend());
    if (m_midiOut == nullptr) {
        qFatal("Unable to find a suitable output backend. You may need to set the DRUMSTICKRT environment variable.");
    }

    drumstick::widgets::SettingsFactory settings;
    if (m_midiIn != nullptr) {
        connect(m_midiIn, &MIDIInput::midiNoteOn,
                this, QOverload<int,int,int>::of(&VPiano::slotNoteOn),
                Qt::QueuedConnection);
        connect(m_midiIn, &MIDIInput::midiNoteOff,
                this, QOverload<int,int,int>::of(&VPiano::slotNoteOff),
                Qt::QueuedConnection);

        if (m_midiIn != nullptr) {
            m_midiIn->initialize(settings.getQSettings());
            MIDIConnection conn;
            auto conin = m_midiIn->connections(VPianoSettings::instance()->advanced());
            auto lastIn = VPianoSettings::instance()->lastInputConnection();
            auto itr = std::find_if(conin.constBegin(), conin.constEnd(), [lastIn](const MIDIConnection& s) { return s.first == lastIn; });
            if(itr == conin.constEnd()) {
                if (!conin.isEmpty()) {
                    conn = conin.first();
                }
            } else {
                conn = (*itr);
            }
            m_midiIn->open(conn);
            auto metaObj = m_midiIn->metaObject();
            if ((metaObj->indexOfProperty("status") != -1) &&
                (metaObj->indexOfProperty("diagnostics") != -1)) {
                auto status = m_midiIn->property("status");
                if (status.isValid() && !status.toBool()) {
                    auto diagnostics = m_midiIn->property("diagnostics");
                    if (diagnostics.isValid()) {
                        auto text = diagnostics.toStringList().join(QChar::LineFeed).trimmed();
                        qWarning() << "MIDI Input" << text;
                    }
                }
            }
        }
    }

    if (m_midiOut != nullptr) {
        m_midiOut->initialize(settings.getQSettings());
        MIDIConnection conn;
        auto connOut = m_midiOut->connections(VPianoSettings::instance()->advanced());
        auto lastOut = VPianoSettings::instance()->lastOutputConnection();
        auto itr = std::find_if(connOut.constBegin(), connOut.constEnd(), [lastOut](const MIDIConnection& s) { return s.first == lastOut; });
        if(itr == connOut.constEnd()) {
            if (!connOut.isEmpty()) {
                conn = connOut.first();
            }
        } else {
            conn = (*itr);
        }
        m_midiOut->open(conn);
        auto metaObj = m_midiOut->metaObject();
        if ((metaObj->indexOfProperty("status") != -1) &&
            (metaObj->indexOfProperty("diagnostics") != -1)) {
            auto status = m_midiOut->property("status");
            if (status.isValid() && !status.toBool()) {
                auto diagnostics = m_midiOut->property("diagnostics");
                if (diagnostics.isValid()) {
                    auto text = diagnostics.toStringList().join(QChar::LineFeed).trimmed();
                    qWarning() << "MIDI Output" << text;
                }
            }
        }
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
        ui.pianokeybd->setChannel(VPianoSettings::instance()->outChannel());
        ui.pianokeybd->setVelocity(VPianoSettings::instance()->velocity());
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

    LabelNaming namingPolicy = VPianoSettings::instance()->namingPolicy();
    switch(namingPolicy) {
    case StandardNames:
        ui.actionStandard->setChecked(true);
        ui.pianokeybd->useStandardNoteNames();
        break;
    case CustomNamesWithSharps:
        ui.actionCustom_Sharps->setChecked(true);
        ui.pianokeybd->useCustomNoteNames(VPianoSettings::instance()->names_sharps());
        break;
    case CustomNamesWithFlats:
        ui.actionCustom_Flats->setChecked(true);
        ui.pianokeybd->useCustomNoteNames(VPianoSettings::instance()->names_flats());
        break;
    }

    LabelOrientation nOrientation = VPianoSettings::instance()->namesOrientation();
    ui.pianokeybd->setLabelOrientation(nOrientation);
    switch(nOrientation) {
    case HorizontalOrientation:
        ui.actionHorizontal->setChecked(true);
        break;
    case VerticalOrientation:
        ui.actionVertical->setChecked(true);
        break;
    case AutomaticOrientation:
        ui.actionAutomatic->setChecked(true);
        break;
    default:
        break;
    }

    LabelAlteration alteration = VPianoSettings::instance()->alterations();
    ui.pianokeybd->setLabelAlterations(alteration);
    switch(alteration) {
    case ShowSharps:
        ui.actionSharps->setChecked(true);
        break;
    case ShowFlats:
        ui.actionFlats->setChecked(true);
        break;
    case ShowNothing:
        ui.actionNothing->setChecked(true);
        break;
    default:
        break;
    }

    LabelVisibility visibility = VPianoSettings::instance()->namesVisibility();
    ui.pianokeybd->setShowLabels(visibility);
    switch(visibility) {
    case ShowNever:
        ui.actionNever->setChecked(true);
        break;
    case ShowMinimum:
        ui.actionMinimal->setChecked(true);
        break;
    case ShowActivated:
        ui.actionWhen_Activated->setChecked(true);
        break;
    case ShowAlways:
        ui.actionAlways->setChecked(true);
        break;
    default:
        break;
    }

    LabelCentralOctave nOctave = VPianoSettings::instance()->namesOctave();
    ui.pianokeybd->setLabelOctave(nOctave);
    switch(nOctave) {
    case OctaveNothing:
        ui.actionNoOctaves->setChecked(true);
        break;
    case OctaveC3:
        ui.actionC3->setChecked(true);
        break;
    case OctaveC4:
        ui.actionC4->setChecked(true);
        break;
    case OctaveC5:
        ui.actionC5->setChecked(true);
        break;
    default:
        break;
    }

    bool octaveSubscript = VPianoSettings::instance()->octaveSubscript();
    ui.pianokeybd->setOctaveSubscript(octaveSubscript);
    ui.actionOctave_Subscript_Designation->setChecked(octaveSubscript);

    ui.statusBar->show();

    ui.pianokeybd->setBaseOctave(VPianoSettings::instance()->baseOctave());
    ui.pianokeybd->setNumKeys(VPianoSettings::instance()->numKeys(), VPianoSettings::instance()->startingKey());

    /*
     * PianoKeybd::setKeyPressedColor(red) is equivalent to:
     *   PianoPalette hpalette(PAL_SINGLE);
     *   hpalette.setColor(0, Qt::red);
     *   ui.pianokeybd->setHighlightPalette(hpalette);
     */
    ui.pianokeybd->setKeyPressedColor(Qt::red);
    ui.pianokeybd->setVelocityTint(false);

    ui.actionInverted_Keys_Color->setChecked(VPianoSettings::instance()->invertedKeys());
    slotInvertedColors(ui.actionInverted_Keys_Color->isChecked());

    ui.actionRaw_Computer_Keyboard->setChecked(VPianoSettings::instance()->rawKeyboard());
    slotRawKeyboard(ui.actionRaw_Computer_Keyboard->isChecked());

    ui.actionComputer_Keyboard_Input->setChecked(VPianoSettings::instance()->keyboardInput());
    slotKeyboardInput(ui.actionComputer_Keyboard_Input->isChecked());

    ui.actionMouse_Input->setChecked(VPianoSettings::instance()->mouseInput());
    slotMouseInput(ui.actionMouse_Input->isChecked());

    ui.actionTouch_Screen_Input->setChecked(VPianoSettings::instance()->touchScreenInput());
    slotTouchScreenInput(ui.actionTouch_Screen_Input->isChecked());
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
    if (ui.pianokeybd->labelAlterations() == ShowFlats) {
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
        VPianoSettings::instance()->setNamesOrientation(HorizontalOrientation);
    } else if(action == ui.actionVertical) {
        VPianoSettings::instance()->setNamesOrientation(VerticalOrientation);
    } else if(action == ui.actionAutomatic) {
        VPianoSettings::instance()->setNamesOrientation(AutomaticOrientation);
    }
    ui.pianokeybd->setLabelOrientation(VPianoSettings::instance()->namesOrientation());
}

void VPiano::slotNameVisibility(QAction* action)
{
    if(action == ui.actionNever) {
        VPianoSettings::instance()->setNamesVisibility(ShowNever);
    } else if(action == ui.actionMinimal) {
        VPianoSettings::instance()->setNamesVisibility(ShowMinimum);
    } else if(action == ui.actionWhen_Activated) {
        VPianoSettings::instance()->setNamesVisibility(ShowActivated);
    } else if(action == ui.actionAlways) {
        VPianoSettings::instance()->setNamesVisibility(ShowAlways);
    }
    ui.pianokeybd->setShowLabels(VPianoSettings::instance()->namesVisibility());
}

void VPiano::slotNameVariant(QAction* action)
{
    if(action == ui.actionSharps) {
        VPianoSettings::instance()->setNamesAlterations(ShowSharps);
    } else if(action == ui.actionFlats) {
        VPianoSettings::instance()->setNamesAlterations(ShowFlats);
    } else if(action == ui.actionNothing) {
        VPianoSettings::instance()->setNamesAlterations(ShowNothing);
    }
    ui.pianokeybd->setLabelAlterations(VPianoSettings::instance()->alterations());
}

void VPiano::slotCentralOctave(QAction *action)
{
    if (action == ui.actionNoOctaves) {
        VPianoSettings::instance()->setNamesOctave(OctaveNothing);
    } else if (action == ui.actionC3) {
        VPianoSettings::instance()->setNamesOctave(OctaveC3);
    } else if(action == ui.actionC4) {
        VPianoSettings::instance()->setNamesOctave(OctaveC4);
    } else if(action == ui.actionC5) {
        VPianoSettings::instance()->setNamesOctave(OctaveC5);
    }
    ui.pianokeybd->setLabelOctave(VPianoSettings::instance()->namesOctave());
}

void VPiano::slotStandardNames()
{
    VPianoSettings::instance()->setNamingPolicy(StandardNames);
    ui.pianokeybd->useStandardNoteNames();
    ui.actionStandard->setChecked(true);
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
            VPianoSettings::instance()->setNamingPolicy(CustomNamesWithSharps);
        } else {
            VPianoSettings::instance()->setNames_flats(customNames);
            VPianoSettings::instance()->setNamingPolicy(CustomNamesWithFlats);
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

void VPiano::slotInvertedColors(bool checked)
{
    PianoPalette bgpal(PAL_KEYS);
    PianoPalette fpal(PAL_FONT);
    if (checked) {
        bgpal.setColor(0, Qt::black);
        bgpal.setColor(1, Qt::white);

        fpal.setColor(0, Qt::white);
        fpal.setColor(1, Qt::black);
        fpal.setColor(2, Qt::white);
        fpal.setColor(3, Qt::white);
    } else {
        bgpal.setColor(0, QColor("ivory"));
        bgpal.setColor(1, QColor(0x40,0x10,0x10));

        fpal.setColor(0, Qt::black);
        fpal.setColor(1, Qt::white);
        fpal.setColor(2, Qt::white);
        fpal.setColor(3, Qt::white);
    }
    ui.pianokeybd->setBackgroundPalette(bgpal);
    ui.pianokeybd->setForegroundPalette(fpal);
    VPianoSettings::instance()->setInvertedKeys(checked);
}

void VPiano::slotRawKeyboard(bool checked)
{
    //qDebug() << Q_FUNC_INFO << checked;
    if (checked) {
        ui.pianokeybd->resetRawKeyboardMap();
    } else {
        ui.pianokeybd->resetKeyboardMap();
    }
    ui.pianokeybd->setRawKeyboardMode(checked);
    VPianoSettings::instance()->setRawKeyboard(checked);
}

void VPiano::slotKeyboardInput(bool checked)
{
    //qDebug() << Q_FUNC_INFO << checked;
    ui.pianokeybd->setKeyboardEnabled(checked);
    VPianoSettings::instance()->setKeyboardInput(checked);
}

void VPiano::slotMouseInput(bool checked)
{
    //qDebug() << Q_FUNC_INFO << checked;
    ui.pianokeybd->setMouseEnabled(checked);
    VPianoSettings::instance()->setMouseInput(checked);
}

void VPiano::slotTouchScreenInput(bool checked)
{
    //qDebug() << Q_FUNC_INFO << checked;
    ui.pianokeybd->setTouchEnabled(checked);
    VPianoSettings::instance()->setTouchScreenInput(checked);
}

void VPiano::slotOctaveSubscript(bool checked)
{
    ui.pianokeybd->setOctaveSubscript(checked);
    VPianoSettings::instance()->setOctaveSubscript(checked);
}
