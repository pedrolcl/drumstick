/*
 Virtual Piano test using the MIDI Sequencer C++ library
 Copyright (C) 2006-2020 Pedro Lopez-Cabanillas <plcl@users.sf.net>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QFileInfo>
#include <QDir>
#include <drumstick/settingsfactory.h>
#include <drumstick/backendmanager.h>
#include "vpianosettings.h"

using namespace drumstick::rt;
using namespace drumstick::widgets;

VPianoSettings::VPianoSettings(QObject *parent) : QObject(parent)
{
    m_nativeInput = QLatin1Literal("Network");
    m_defaultInput = QLatin1Literal("21928");
#if defined(Q_OS_LINUX)
    m_nativeOutput = QLatin1Literal("SonivoxEAS");
    m_defaultOutput = QLatin1Literal("SonivoxEAS");
#elif defined(Q_OS_MACOS)
    m_nativeOutput = QLatin1Literal("DLS Synth");
    m_defaultOutput = QLatin1Literal("DLS Synth");
#elif defined(Q_OS_WIN)
    m_nativeOutput = QLatin1Literal("Windows MM");
    m_defaultOutput = QLatin1Literal("Microsoft GS Wavetable Synth");
#else
    m_nativeOutput = m_nativeInput;
    m_defaultOutput = m_defaultInput;
#endif
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
        { drumstick::rt::QSTR_DRUMSTICKRT_PUBLICNAMEIN, QLatin1String("Virtual Piano IN")},
        { drumstick::rt::QSTR_DRUMSTICKRT_PUBLICNAMEOUT, QLatin1String("Virtual Piano OUT")}
    };
    emit ValuesChanged();
}

void VPianoSettings::ReadSettings()
{
    SettingsFactory settings;
    internalRead(*settings);
}

void VPianoSettings::SaveSettings()
{
    SettingsFactory settings;
    internalSave(*settings);
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
    settings.beginGroup("Window");
    m_geometry = settings.value("Geometry").toByteArray();
    m_state = settings.value("State").toByteArray();
    settings.endGroup();

    settings.beginGroup(QSTR_DRUMSTICKRT_GROUP);
    QStringList keys = settings.allKeys();
    for(const QString& key : m_defaultsMap.keys()) {
        if (!keys.contains(key)) {
            keys.append(key);
        }
    }
    for(const QString& key : keys) {
        QVariant defval = m_defaultsMap.contains(key) ? m_defaultsMap[key] : QString();
        m_settingsMap.insert(key, settings.value(key, defval));
    }
    settings.endGroup();

    settings.beginGroup("Connections");
    m_lastInputBackend = settings.value("inputBackend", m_nativeInput).toString();
    m_lastOutputBackend = settings.value("outputBackend", m_nativeOutput).toString();
    m_lastInputConnection = settings.value("inputConnection", m_defaultInput).toString();
    m_lastOutputConnection = settings.value("outputConnection", m_defaultOutput).toString();
    m_midiThru = settings.value("midiThru", false).toBool();
    m_advanced = settings.value("advanced", false).toBool();
    settings.endGroup();

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
    settings.endGroup();

    emit ValuesChanged();
}

void VPianoSettings::internalSave(QSettings &settings)
{
    settings.beginGroup("Window");
    settings.setValue("Geometry", m_geometry);
    settings.setValue("State", m_state);
    settings.endGroup();

    settings.beginGroup(QSTR_DRUMSTICKRT_GROUP);
    for(auto key : m_settingsMap.keys()) {
        settings.setValue(key, m_settingsMap[key]);
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
    settings.endGroup();
    settings.sync();
}

QVariantMap VPianoSettings::settingsMap() const
{
    return m_settingsMap;
}

QString VPianoSettings::nativeOutput() const
{
    return m_nativeOutput;
}

QString VPianoSettings::nativeInput() const
{
    return m_nativeInput;
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
