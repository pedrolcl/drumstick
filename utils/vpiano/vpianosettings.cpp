/*
 Virtual Piano test using the MIDI Sequencer C++ library
 Copyright (C) 2006-2024 Pedro Lopez-Cabanillas <plcl@users.sf.net>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "vpianosettings.h"
#include <QDir>
#include <QFileInfo>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
    #include <QTouchDevice>
#else
    #include <QInputDevice>
#endif
#include <drumstick/settingsfactory.h>

using namespace drumstick::rt;
using namespace drumstick::widgets;

VPianoSettings::VPianoSettings(QObject *parent) : QObject(parent)
{
    ResetDefaults();
}

VPianoSettings* VPianoSettings::instance()
{
    static VPianoSettings inst;
    return &inst;
}

void VPianoSettings::ResetDefaults()
{
    m_midiThru = false;
    m_advanced = false;
    m_inChannel = 0;
    m_outChannel = 0;
    m_velocity = 100;
    m_baseOctave = 1;
    m_numKeys = 88;
    m_startingKey = 9;
    m_defaultsMap = QVariantMap{
        { BackendManager::QSTR_DRUMSTICKRT_PUBLICNAMEIN, QStringLiteral("Virtual Piano IN")},
        { BackendManager::QSTR_DRUMSTICKRT_PUBLICNAMEOUT, QStringLiteral("Virtual Piano OUT")}
    };
    Q_EMIT ValuesChanged();
}

void VPianoSettings::ReadSettings()
{
    SettingsFactory settings;
    internalRead(*settings.getQSettings());
}

void VPianoSettings::SaveSettings()
{
    SettingsFactory settings;
    internalSave(*settings.getQSettings());
}

void VPianoSettings::ReadFromFile(const QString &filepath)
{
    QSettings settings(filepath, QSettings::IniFormat);
    internalRead(settings);
}

void VPianoSettings::SaveToFile(const QString &filepath)
{
    QSettings settings(filepath, QSettings::IniFormat);
    internalSave(settings);
}

void VPianoSettings::internalRead(QSettings &settings)
{
    const QStringList STD_NAMES_S{"do", "do♯", "re", "re♯", "mi", "fa", "fa♯", "sol", "sol♯", "la", "la♯", "si"};
    const QStringList STD_NAMES_F{"do", "re♭", "re", "mi♭", "mi", "fa", "sol♭", "sol", "la♭", "la", "si♭", "si"};

    settings.beginGroup("Window");
    m_geometry = settings.value("Geometry").toByteArray();
    m_state = settings.value("State").toByteArray();
    settings.endGroup();

    settings.beginGroup(BackendManager::QSTR_DRUMSTICKRT_GROUP);
    QStringList keys = settings.allKeys();
    for(auto it = m_defaultsMap.begin(); it != m_defaultsMap.end(); ++it) {
        if (!keys.contains(it.key())) {
            keys.append(it.key());
        }
    }
    for (const QString &key : std::as_const(keys)) {
        QVariant defval = m_defaultsMap.contains(key) ? m_defaultsMap[key] : QString();
        m_settingsMap.insert(key, settings.value(key, defval));
    }
    settings.endGroup();

    settings.beginGroup("Connections");
    m_lastInputBackend = settings.value("inputBackend").toString();
    m_lastOutputBackend = settings.value("outputBackend").toString();
    m_lastInputConnection = settings.value("inputConnection").toString();
    m_lastOutputConnection = settings.value("outputConnection").toString();
    m_midiThru = settings.value("midiThru", false).toBool();
    m_advanced = settings.value("advanced", false).toBool();
    settings.endGroup();

    bool touchInputEnabled = false;
    bool mouseInputEnabled = true;
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
    const QList<const QTouchDevice*> devs = QTouchDevice::devices();
    for(const QTouchDevice *dev : devs) {
        if (dev->type() == QTouchDevice::TouchScreen) {
            /*qDebug() << "platform:" << qApp->platformName()
                     << "desktop:" << qgetenv("XDG_SESSION_DESKTOP")
                     << "touch device:" << dev;*/
            touchInputEnabled = true;
            mouseInputEnabled = !dev->capabilities().testFlag(QTouchDevice::MouseEmulation);
            break;
        }
    }
#else
    foreach(const QInputDevice *dev, QInputDevice::devices()) {
        if (dev->type() == QInputDevice::DeviceType::TouchScreen) {
            /*qDebug() << "platform:" << qApp->platformName()
                     << "desktop:" << qgetenv("XDG_SESSION_DESKTOP")
                     << "touch device:" << dev;*/
            touchInputEnabled = true;
            mouseInputEnabled = !dev->capabilities().testFlag(QInputDevice::Capability::MouseEmulation);
            break;
        }
    }
#endif

    settings.beginGroup("Preferences");
    setInChannel(settings.value("inputChannel", 0).toInt());
    setOutChannel(settings.value("outputChannel", 0).toInt());
    setVelocity(settings.value("velocity", 100).toInt());
    setBaseOctave(settings.value("baseOctave", 1).toInt());
    setNumKeys(settings.value("numKeys", 88).toInt());
    setStartingKey(settings.value("startingKey", 9).toInt());
    setBaseOctave(settings.value("baseOctave", 1).toInt());
    setNumKeys(settings.value("numKeys", 88).toInt());
    setStartingKey(settings.value("startingKey", 9).toInt());
    setRawKeyboard(settings.value("raw_keyboard", false).toBool());
    setKeyboardInput(settings.value("keyboard_input", true).toBool());
    setMouseInput(settings.value("mouse_input", mouseInputEnabled).toBool());
    setTouchScreenInput(settings.value("touchscreen_input", touchInputEnabled).toBool());
    setOctaveSubscript(settings.value("octave_subscript", true).toBool());
    settings.endGroup();

    settings.beginGroup("TextSettings");
    QString defaultFont = QGuiApplication::font().family() + ",50";
    QFont f;
    if (f.fromString(settings.value("namesFont", defaultFont).toString())) {
        setNamesFont(f);
    }
    setNamesOrientation(static_cast<LabelOrientation>(settings.value("namesOrientation", HorizontalOrientation).toInt()));
    setNamesVisibility(static_cast<LabelVisibility>(settings.value("namesVisibility", ShowNever).toInt()));
    setNamesAlterations(static_cast<LabelAlteration>(settings.value("namesAlteration", ShowSharps).toInt()));
    setNamingPolicy(static_cast<LabelNaming>(settings.value("namingPolicy", StandardNames).toInt()));
    setNamesOctave(static_cast<LabelCentralOctave>(settings.value("namesOctave", OctaveC4).toInt()));
    setNames_sharps(settings.value("names_sharps", STD_NAMES_S).toStringList());
    setNames_flats(settings.value("names_flats", STD_NAMES_F).toStringList());
    setInvertedKeys(settings.value("inverted_keys", false).toBool());
    settings.endGroup();

    Q_EMIT ValuesChanged();
}

void VPianoSettings::internalSave(QSettings &settings)
{
    settings.beginGroup("Window");
    settings.setValue("Geometry", m_geometry);
    settings.setValue("State", m_state);
    settings.endGroup();

    settings.beginGroup(BackendManager::QSTR_DRUMSTICKRT_GROUP);
    for (auto it = m_settingsMap.begin(); it != m_settingsMap.end(); ++it) {
        settings.setValue(it.key(), it.value());
    }
    settings.endGroup();

    settings.beginGroup("Connections");
    settings.setValue("inputBackend", m_lastInputBackend);
    settings.setValue("outputBackend", m_lastOutputBackend);
    settings.setValue("inputConnection", m_lastInputConnection);
    settings.setValue("outputConnection", m_lastOutputConnection);
    settings.setValue("midiThru", m_midiThru);
    settings.setValue("advanced", m_advanced);
    settings.endGroup();

    settings.beginGroup("Preferences");
    settings.setValue("inputChannel", m_inChannel);
    settings.setValue("outputChannel", m_outChannel);
    settings.setValue("velocity", m_velocity);
    settings.setValue("baseOctave", m_baseOctave);
    settings.setValue("numKeys", m_numKeys);
    settings.setValue("startingKey", m_startingKey);
    settings.setValue("raw_keyboard", m_rawKeyboard);
    settings.setValue("keyboard_input", m_keyboardInput);
    settings.setValue("mouse_input", m_mouseInput);
    settings.setValue("touchscreen_input", m_touchScreenInput);
    settings.setValue("octave_subscript", m_octaveSubscript);
    settings.endGroup();

    settings.beginGroup("TextSettings");
    settings.setValue("namesFont", fontString(m_namesFont));
    settings.setValue("namesOrientation", m_namesOrientation);
    settings.setValue("namesVisibility", m_namesVisibility);
    settings.setValue("namesAlteration", m_namesAlteration);
    settings.setValue("namingPolicy", m_namingPolicy);
    settings.setValue("namesOctave", m_namesOctave);
    settings.setValue("names_sharps", m_names_sharps);
    settings.setValue("names_flats", m_names_flats);
    settings.setValue("inverted_keys", m_invertedKeys);
    settings.endGroup();

    settings.sync();
}

QString VPianoSettings::fontString(const QFont &f) const
{
    return QString("%1,%2").arg(f.family()).arg(f.pointSize());
}

bool VPianoSettings::octaveSubscript() const
{
    return m_octaveSubscript;
}

void VPianoSettings::setOctaveSubscript(bool newOctaveSubscript)
{
    m_octaveSubscript = newOctaveSubscript;
}

bool VPianoSettings::touchScreenInput() const
{
    return m_touchScreenInput;
}

void VPianoSettings::setTouchScreenInput(bool newTouchScreenInput)
{
    m_touchScreenInput = newTouchScreenInput;
}

bool VPianoSettings::mouseInput() const
{
    return m_mouseInput;
}

void VPianoSettings::setMouseInput(bool newMouseInput)
{
    m_mouseInput = newMouseInput;
}

bool VPianoSettings::keyboardInput() const
{
    return m_keyboardInput;
}

void VPianoSettings::setKeyboardInput(bool newKeyboardInput)
{
    m_keyboardInput = newKeyboardInput;
}

bool VPianoSettings::rawKeyboard() const
{
    return m_rawKeyboard;
}

void VPianoSettings::setRawKeyboard(bool newRawKeyboard)
{
    m_rawKeyboard = newRawKeyboard;
}

bool VPianoSettings::invertedKeys() const
{
    return m_invertedKeys;
}

void VPianoSettings::setInvertedKeys(bool newInvertedKeys)
{
    m_invertedKeys = newInvertedKeys;
}

QStringList VPianoSettings::names_flats() const
{
    return m_names_flats;
}

void VPianoSettings::setNames_flats(const QStringList &names_flats)
{
    m_names_flats = names_flats;
}

QStringList VPianoSettings::names_sharps() const
{
    return m_names_sharps;
}

void VPianoSettings::setNames_sharps(const QStringList &names_sharps)
{
    m_names_sharps = names_sharps;
}

LabelNaming VPianoSettings::namingPolicy() const
{
    return m_namingPolicy;
}

void VPianoSettings::setNamingPolicy(const LabelNaming namingPolicy)
{
    m_namingPolicy = namingPolicy;
}

LabelCentralOctave VPianoSettings::namesOctave() const
{
    return m_namesOctave;
}

void VPianoSettings::setNamesOctave(const LabelCentralOctave namesOctave)
{
    m_namesOctave = namesOctave;
}

QFont VPianoSettings::namesFont() const
{
    return m_namesFont;
}

void VPianoSettings::setNamesFont(const QFont &namesFont)
{
    m_namesFont = namesFont;
}

LabelAlteration VPianoSettings::alterations() const
{
    return m_namesAlteration;
}

void VPianoSettings::setNamesAlterations(const LabelAlteration alterations)
{
    m_namesAlteration = alterations;
}

LabelVisibility VPianoSettings::namesVisibility() const
{
    return m_namesVisibility;
}

void VPianoSettings::setNamesVisibility(const LabelVisibility namesVisibility)
{
    m_namesVisibility = namesVisibility;
}

LabelOrientation VPianoSettings::namesOrientation() const
{
    return m_namesOrientation;
}

void VPianoSettings::setNamesOrientation(const LabelOrientation namesOrientation)
{
    m_namesOrientation = namesOrientation;
}

QVariantMap VPianoSettings::settingsMap() const
{
    return m_settingsMap;
}

int VPianoSettings::startingKey() const
{
    return m_startingKey;
}

void VPianoSettings::setStartingKey(int startingKey)
{
    m_startingKey = startingKey;
}

int VPianoSettings::numKeys() const
{
    return m_numKeys;
}

void VPianoSettings::setNumKeys(int numKeys)
{
    m_numKeys = numKeys;
}

int VPianoSettings::baseOctave() const
{
    return m_baseOctave;
}

void VPianoSettings::setBaseOctave(int baseOctave)
{
    m_baseOctave = baseOctave;
}

int VPianoSettings::velocity() const
{
    return m_velocity;
}

void VPianoSettings::setVelocity(int velocity)
{
    m_velocity = velocity;
}

int VPianoSettings::outChannel() const
{
    return m_outChannel;
}

void VPianoSettings::setOutChannel(int outChannel)
{
    m_outChannel = outChannel;
}

int VPianoSettings::inChannel() const
{
    return m_inChannel;
}

void VPianoSettings::setInChannel(int inChannel)
{
    m_inChannel = inChannel;
}

bool VPianoSettings::advanced() const
{
    return m_advanced;
}

void VPianoSettings::setAdvanced(bool advanced)
{
    m_advanced = advanced;
}

bool VPianoSettings::midiThru() const
{
    return m_midiThru;
}

void VPianoSettings::setMidiThru(bool midiThru)
{
    m_midiThru = midiThru;
}

QString VPianoSettings::lastOutputConnection() const
{
    return m_lastOutputConnection;
}

void VPianoSettings::setLastOutputConnection(const QString &lastOutputConnection)
{
    m_lastOutputConnection = lastOutputConnection;
}

QString VPianoSettings::lastInputConnection() const
{
    return m_lastInputConnection;
}

void VPianoSettings::setLastInputConnection(const QString &lastInputConnection)
{
    m_lastInputConnection = lastInputConnection;
}

QString VPianoSettings::lastOutputBackend() const
{
    return m_lastOutputBackend;
}

void VPianoSettings::setLastOutputBackend(const QString &lastOutputBackend)
{
    m_lastOutputBackend = lastOutputBackend;
}

QString VPianoSettings::lastInputBackend() const
{
    return m_lastInputBackend;
}

void VPianoSettings::setLastInputBackend(const QString &lastInputBackend)
{
    m_lastInputBackend = lastInputBackend;
}

QByteArray VPianoSettings::state() const
{
    return m_state;
}

void VPianoSettings::setState(const QByteArray &state)
{
    m_state = state;
}

QByteArray VPianoSettings::geometry() const
{
    return m_geometry;
}

void VPianoSettings::setGeometry(const QByteArray &geometry)
{
    m_geometry = geometry;
}
