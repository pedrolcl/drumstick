/*
    Drumstick RT (realtime MIDI In/Out)
    Copyright (C) 2009-2014 Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; If not, see <http://www.gnu.org/licenses/>.
*/

#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QDesktopServices>
#include <QStandardPaths>
#include <QDebug>

#include "synthengine.h"

const QString QSTR_PREFERENCES("Preferences");
const QString QSTR_INSTRUMENTSDEFINITION("InstrumentsDefinition");
const QString QSTR_DATADIR("soundfonts");
const QString QSTR_SOUNDFONT("default.sf2");

SynthEngine::SynthEngine(QObject *parent)
    : QObject(parent)
{ }

SynthEngine::~SynthEngine()
{
    ::delete_fluid_audio_driver(m_driver);
    ::delete_fluid_synth(m_synth);
    ::delete_fluid_settings(m_settings);
}

void SynthEngine::initializeSynth(QSettings* settings)
{
    Q_UNUSED(settings)
    m_settings = ::new_fluid_settings();
#if defined(Q_OS_LINUX)
    ::fluid_settings_setstr(m_settings, "audio.driver", "pulseaudio");
#elif defined(Q_OS_WIN)
    ::fluid_settings_setstr(m_settings, "audio.driver", "dsound");
#elif defined(Q_OS_OSX)
    ::fluid_settings_setstr(m_settings, "audio.driver", "coreaudio");
#endif
    ::fluid_settings_setint(m_settings, "audio.period-size", 256);
    ::fluid_settings_setint(m_settings, "audio.periods", 2);
    ::fluid_settings_setnum(m_settings, "synth.sample-rate", 48000.0);
    ::fluid_settings_setint(m_settings, "synth.chorus.active", 0);
    ::fluid_settings_setint(m_settings, "synth.reverb.active", 0);
    ::fluid_settings_setnum(m_settings, "synth.gain", 0.4);
    ::fluid_settings_setint(m_settings, "synth.polyphony", 16);
    m_synth = ::new_fluid_synth(m_settings);
    m_driver = ::new_fluid_audio_driver(m_settings, m_synth);
    ::fluid_synth_set_chorus_on(m_synth, 0);
    ::fluid_synth_set_reverb_on(m_synth, 0);
    ::fluid_synth_set_interp_method(m_synth, -1, FLUID_INTERP_DEFAULT);
}

void SynthEngine::setInstrument(int channel, int pgm)
{
    //Preset *bp = dynamic_cast<Preset*>(m_instruments.at(i));
    //if (bp != 0) {
        //qDebug() << i << bp->name() << bp->bank() << bp->program();
        //::fluid_synth_bank_select(m_synth, channel, bp->bank());
        //::fluid_synth_program_change(m_synth, channel, bp->program());
    //}
    ::fluid_synth_program_change(m_synth, channel, pgm);
}
void SynthEngine::noteOn(int channel, int midiNote, int velocity)
{
    //qDebug() << "NoteOn " << midiNote << " vel: " << velocity; // << " time: " << m_time.elapsed();
    ::fluid_synth_noteon(m_synth, channel, midiNote, velocity);
}

void SynthEngine::noteOff(int channel, int midiNote, int /*velocity*/)
{
    //qDebug() << "NoteOff " << midiNote << " vel: " << velocity; // << " time: " << m_time.elapsed();
    ::fluid_synth_noteoff(m_synth, channel, midiNote);
}

void SynthEngine::loadSoundFont()
{
    //fluid_preset_t preset;
    //m_instruments.clear();
    if (m_sfid != -1) {
        ::fluid_synth_sfunload(m_synth, unsigned(m_sfid), 1);
    }
    m_sfid = ::fluid_synth_sfload(m_synth, m_soundFont.toLocal8Bit(), 1);
    /*if (m_sfid != -1) {
        fluid_sfont_t *pSoundFont = ::fluid_synth_get_sfont_by_id(m_synth, unsigned(m_sfid));
        if (pSoundFont) {
            pSoundFont->iteration_start(pSoundFont);
            while (pSoundFont->iteration_next(pSoundFont, &preset)) {
                //int iBank = preset.get_banknum(&preset);
                //int iProg = preset.get_num(&preset);
                QString sName = preset.get_name(&preset);
                //qDebug() << iBank << iProg << sName;
                //Preset *bp = new Preset(iBank, iProg, sName, this);
                //m_instruments.append(bp);
            }
        }
    }*/
}

void SynthEngine::initialize()
{
    qDebug() << Q_FUNC_INFO;
    initializeSynth();
    scanSoundFonts();
    loadSoundFont();
    if (m_sfid < 0) {
        m_soundFont = m_defSoundFont;
        loadSoundFont();
    }
}

QString SynthEngine::currentInstrumentName(int channel)
{
    unsigned int sfid, bank, preset;
    ::fluid_synth_get_program(m_synth, channel, &sfid, &bank, &preset);
    /*foreach(Preset *bp, m_instruments) {
        if (bp->bank() == bank && bp->program() == preset)
            return bp->name();
    }*/
    return QLatin1String("Select...");
}

void SynthEngine::panic()
{
    ::fluid_synth_system_reset(m_synth);
}

void SynthEngine::controlChange(const int channel, const int midiCtl, const int value)
{
    ::fluid_synth_cc(m_synth, channel, midiCtl, value);
}

void SynthEngine::bender(const int channel, const int value)
{
    ::fluid_synth_pitch_bend(m_synth, channel, value + 8192);
}

void SynthEngine::setSoundFont(const QString &value)
{
    //qDebug() << Q_FUNC_INFO << value;
    if (value != m_soundFont) {
        m_soundFont = value;
        loadSoundFont();
    }
}

void SynthEngine::scanSoundFonts(const QDir &initialDir)
{
    //qDebug() << Q_FUNC_INFO << initialDir.absolutePath();
    QDir dir(initialDir);
    dir.setFilter(QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Name);
    QStringList filters;
    filters << "*.sf2" << "*.SF2";
    QFileInfoList entries= dir.entryInfoList(filters);
    foreach(const QFileInfo &info, entries) {
        QString name = info.absoluteFilePath();
        if (info.isFile()) {
            m_soundFontsList << name;
        } else if (info.isDir()){
            scanSoundFonts(name);
        }
    }
}

void SynthEngine::scanSoundFonts()
{
    m_soundFontsList.clear();
    QStringList paths = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    foreach(const QString& p, paths) {
       QDir d(p + QDir::separator() + QSTR_DATADIR);
       if (d.exists()) {
            scanSoundFonts(d);
       }
    }
}

void SynthEngine::readSettings(QSettings *settings)
{
    qDebug() << Q_FUNC_INFO;
    QDir dir(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QSTR_DATADIR, QStandardPaths::LocateDirectory));
    QFileInfo sf2(dir, QSTR_SOUNDFONT);
    if (sf2.exists()) {
        m_defSoundFont = sf2.absoluteFilePath();
    }
    m_sfid = -1;

    settings->beginGroup(QSTR_PREFERENCES);
    m_soundFont = settings->value(QSTR_INSTRUMENTSDEFINITION, m_defSoundFont).toString();
    settings->endGroup();
}

void SynthEngine::saveSettings(QSettings *settings)
{
    qDebug() << Q_FUNC_INFO;
    settings->beginGroup(QSTR_PREFERENCES);
    settings->setValue(QSTR_INSTRUMENTSDEFINITION, m_soundFont);
    settings->endGroup();
    settings->sync();
}

void SynthEngine::close()
{
    m_currentConnection.clear();
}

void SynthEngine::open()
{
    m_currentConnection = QSTR_FLUIDSYNTH;
}
