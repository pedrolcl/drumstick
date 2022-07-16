/*
    Drumstick RT (realtime MIDI In/Out)
    Copyright (C) 2009-2022 Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <drumstick/rtmidioutput.h>
#include "synthengine.h"

namespace drumstick { namespace rt {

const QString SynthEngine::QSTR_FLUIDSYNTH_VERSION = QStringLiteral(FLUIDSYNTH_VERSION);
const QString SynthEngine::QSTR_FLUIDSYNTH = QStringLiteral("FluidSynth");
const QString SynthEngine::QSTR_PREFERENCES = QStringLiteral("FluidSynth");
const QString SynthEngine::QSTR_INSTRUMENTSDEFINITION = QStringLiteral("InstrumentsDefinition");
const QString SynthEngine::QSTR_DATADIR = QStringLiteral("soundfonts");
const QString SynthEngine::QSTR_DATADIR2 = QStringLiteral("sounds/sf2");
const QString SynthEngine::QSTR_SOUNDFONT = QStringLiteral("default.sf2");

const QString SynthEngine::QSTR_AUDIODRIVER = QStringLiteral("AudioDriver");
const QString SynthEngine::QSTR_BUFFERTIME = QStringLiteral("BufferTime");
const QString SynthEngine::QSTR_PERIODSIZE = QStringLiteral("PeriodSize");
const QString SynthEngine::QSTR_PERIODS = QStringLiteral("Periods");
const QString SynthEngine::QSTR_SAMPLERATE = QStringLiteral("SampleRate");
const QString SynthEngine::QSTR_CHORUS = QStringLiteral("Chorus");
const QString SynthEngine::QSTR_REVERB = QStringLiteral("Reverb");
const QString SynthEngine::QSTR_GAIN = QStringLiteral("Gain");
const QString SynthEngine::QSTR_POLYPHONY = QStringLiteral("Polyphony");

const QString SynthEngine::QSTR_DEFAULT_AUDIODRIVER =
#if defined(Q_OS_LINUX)
    QStringLiteral("pulseaudio");
#elif defined(Q_OS_WIN)
    QStringLiteral("dsound");
#elif defined(Q_OS_OSX)
    QStringLiteral("coreaudio");
#else
    QStringLiteral("oss");
#endif
const int SynthEngine::DEFAULT_PERIODS = 8;
const int SynthEngine::DEFAULT_PERIODSIZE = 512;
const double SynthEngine::DEFAULT_SAMPLERATE = 44100.0;
const int SynthEngine::DEFAULT_CHORUS = 0;
const int SynthEngine::DEFAULT_REVERB = 1;
const double SynthEngine::DEFAULT_GAIN = 1.0;
const int SynthEngine::DEFAULT_POLYPHONY = 256;

static void
SynthEngine_log_function(int level, const char* message, void* data)
{
    SynthEngine *classInstance = static_cast<SynthEngine*>(data);
    classInstance->appendDiagnostics(level, message);
}

SynthEngine::SynthEngine(QObject *parent)
    : QObject(parent),
      m_sfid(0),
      m_settings(nullptr),
      m_synth(nullptr),
      m_driver(nullptr)
{
    //qDebug() << Q_FUNC_INFO; 
    m_runtimeLibraryVersion = ::fluid_version_str();
    //qDebug() << "Compiled FluidSynth Version:" << QSTR_FLUIDSYNTH_VERSION;
    //qDebug() << "Runtime FluidSynth Version:" << m_runtimeLibraryVersion;
    //::fluid_set_log_function(fluid_log_level::FLUID_DBG, &SynthEngine_log_function, this);
    ::fluid_set_log_function(fluid_log_level::FLUID_ERR, &SynthEngine_log_function, this);
    ::fluid_set_log_function(fluid_log_level::FLUID_WARN, &SynthEngine_log_function, this);
    ::fluid_set_log_function(fluid_log_level::FLUID_INFO, &SynthEngine_log_function, this);
}

SynthEngine::~SynthEngine()
{
    //qDebug() << Q_FUNC_INFO;
    uninitialize();
}

void SynthEngine::uninitialize()
{
    //qDebug() << Q_FUNC_INFO;
    if (m_driver != nullptr) {
        ::delete_fluid_audio_driver(m_driver);
        m_driver = nullptr;
    }
    if (m_synth != nullptr) {
        ::delete_fluid_synth(m_synth);
        m_synth = nullptr;
    }
    if (m_settings != nullptr) {
        ::delete_fluid_settings(m_settings);
        m_settings = nullptr;
    }
    m_status = false;
    m_diagnostics.clear();
}

void SynthEngine::initializeSynth()
{
    uninitialize();
    //qDebug() << Q_FUNC_INFO << fs_audiodriver << fs_periodSize << fs_periods << qEnvironmentVariableIntValue("PULSE_LATENCY_MSEC");
    m_settings = ::new_fluid_settings();
    ::fluid_settings_setstr(m_settings, "audio.driver", qPrintable(fs_audiodriver));
    ::fluid_settings_setint(m_settings, "audio.period-size", fs_periodSize);
    ::fluid_settings_setint(m_settings, "audio.periods", fs_periods);
    ::fluid_settings_setint(m_settings, "audio.pulseaudio.adjust-latency", 0);
    ::fluid_settings_setnum(m_settings, "synth.sample-rate", fs_sampleRate);
    ::fluid_settings_setint(m_settings, "synth.chorus.active", fs_chorus);
    ::fluid_settings_setint(m_settings, "synth.reverb.active", fs_reverb);
    ::fluid_settings_setnum(m_settings, "synth.gain", fs_gain);
    ::fluid_settings_setint(m_settings, "synth.polyphony", fs_polyphony);
    m_synth = ::new_fluid_synth(m_settings);
    m_driver = ::new_fluid_audio_driver(m_settings, m_synth);
}

void SynthEngine::setInstrument(int channel, int pgm)
{
    ::fluid_synth_program_change(m_synth, channel, pgm);
}

void SynthEngine::noteOn(int channel, int midiNote, int velocity)
{
    ::fluid_synth_noteon(m_synth, channel, midiNote, velocity);
}

void SynthEngine::noteOff(int channel, int midiNote, int /*velocity*/)
{
    ::fluid_synth_noteoff(m_synth, channel, midiNote);
}

void SynthEngine::loadSoundFont()
{
    if (m_sfid != -1) {
        ::fluid_synth_sfunload(m_synth, unsigned(m_sfid), 1);
    }
    m_sfid = ::fluid_synth_sfload(m_synth, qPrintable(m_soundFont), 1);
}

void SynthEngine::initialize()
{
    //qDebug() << Q_FUNC_INFO;
    initializeSynth();
    retrieveAudioDrivers();
    scanSoundFonts();
    loadSoundFont();
    m_status = (m_synth != nullptr) && (m_driver != nullptr) && (m_sfid >= 0);
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
    if (value != m_soundFont) {
        m_soundFont = value;
        loadSoundFont();
    }
}

void SynthEngine::appendDiagnostics(int level, const char *message)
{
    static const QMap<int,QString> prefix {
        {fluid_log_level::FLUID_DBG,  tr("Debug")},
        {fluid_log_level::FLUID_ERR,  tr("Error")},
        {fluid_log_level::FLUID_WARN, tr("Warning")},
        {fluid_log_level::FLUID_INFO, tr("Information")}
    };
    m_diagnostics.append(prefix[level]+": "+message);
}

void SynthEngine::stop()
{
    //qDebug() << Q_FUNC_INFO;
    uninitialize();
}

QStringList SynthEngine::getAudioDrivers()
{
    return m_audioDriversList;
}

QStringList SynthEngine::getDiagnostics()
{
    return m_diagnostics;
}

QString SynthEngine::getLibVersion()
{
    return m_runtimeLibraryVersion;
}

bool SynthEngine::getStatus()
{
    return m_status;
}


void SynthEngine::scanSoundFonts(const QDir &initialDir)
{
    QDir dir(initialDir);
    dir.setFilter(QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Name);
    QStringList filters;
    filters << "*.sf2" << "*.SF2" << "*.sf3" << "*.SF3" << "*.dls" << "*.DLS";
    QFileInfoList entries= dir.entryInfoList(filters);
    foreach(const QFileInfo &info, entries) {
        QString name = info.absoluteFilePath();
        if (info.isFile() && info.fileName().toLower() == QSTR_SOUNDFONT) {
            m_soundFontsList << name;
        } else if (info.isDir()){
            scanSoundFonts(name);
        }
    }
}

void SynthEngine::retrieveAudioDrivers()
{
    if (m_settings != nullptr) {
        m_audioDriversList.clear();
        ::fluid_settings_foreach_option(m_settings, "audio.driver", &m_audioDriversList, [](void *context2, const char *, const char *option2){
            QStringList *options_list = static_cast<QStringList*>(context2);
            options_list->append(option2);
        });
        //qDebug() << "Drivers: " << m_audioDriversList;
    }
}

void SynthEngine::scanSoundFonts()
{
    m_soundFontsList.clear();
    QStringList paths = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
#if defined(Q_OS_OSX)
    paths << (QCoreApplication::applicationDirPath() + QLatin1String("../Resources"));
#endif
    foreach(const QString& p, paths) {
       QDir d(p + QDir::separator() + QSTR_DATADIR);
       if (!d.exists()) {
           d = QDir(p + QDir::separator() + QSTR_DATADIR2);
       }
       if (d.exists()) {
            scanSoundFonts(d);
       }
    }
    if (m_defSoundFont.isEmpty() && m_soundFontsList.length() > 0) {
        m_defSoundFont = m_soundFontsList.first();
    }
}

void SynthEngine::readSettings(QSettings *settings)
{
    QDir dir;
#if defined(Q_OS_OSX)
    dir = QDir(QCoreApplication::applicationDirPath() + QLatin1String("/../Resources"));
#elif defined(Q_OS_UNIX)
    dir = QDir(QCoreApplication::applicationDirPath() + QLatin1String("/../share/soundfonts/"));
    if (!dir.exists()) {
        dir = QDir(QCoreApplication::applicationDirPath() + QLatin1String("/../share/sounds/sf2/"));
    }
#else
    dir = QDir(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QSTR_DATADIR, QStandardPaths::LocateDirectory));
#endif
    QFileInfo sf2(dir, QSTR_SOUNDFONT);
    if (sf2.exists()) {
        m_defSoundFont = sf2.absoluteFilePath();
    }
    m_sfid = -1;
    //qDebug() << "defSoundFont:" << m_defSoundFont;
    settings->beginGroup(QSTR_PREFERENCES);
    m_soundFont = settings->value(QSTR_INSTRUMENTSDEFINITION, m_defSoundFont).toString();
    fs_audiodriver = settings->value(QSTR_AUDIODRIVER, QSTR_DEFAULT_AUDIODRIVER).toString();
    fs_periodSize = settings->value(QSTR_PERIODSIZE, DEFAULT_PERIODSIZE).toInt();
    fs_periods = settings->value(QSTR_PERIODS, DEFAULT_PERIODS).toInt();
    fs_sampleRate = settings->value(QSTR_SAMPLERATE, DEFAULT_SAMPLERATE).toDouble();
    fs_chorus = settings->value(QSTR_CHORUS, DEFAULT_CHORUS).toInt();
    fs_reverb = settings->value(QSTR_REVERB, DEFAULT_REVERB).toInt();
    fs_gain = settings->value(QSTR_GAIN, DEFAULT_GAIN).toDouble();
    fs_polyphony = settings->value(QSTR_POLYPHONY, DEFAULT_POLYPHONY).toInt();
    settings->endGroup();
    //qDebug() << Q_FUNC_INFO << "audioDriver:" << fs_audiodriver << "buffer" << fs_periodSize << '*' << fs_periods;
    int bufferTime = 1000 * fs_periodSize * fs_periods / fs_sampleRate;
    qputenv("PULSE_LATENCY_MSEC", QByteArray::number( bufferTime ) );
    //qDebug() << Q_FUNC_INFO << "$PULSE_LATENCY_MSEC=" << bufferTime;
}

void SynthEngine::close()
{
    //qDebug() << Q_FUNC_INFO;
    m_currentConnection = MIDIConnection();
}

void SynthEngine::open()
{
    //qDebug() << Q_FUNC_INFO;
    m_currentConnection = MIDIConnection(QSTR_FLUIDSYNTH, QSTR_FLUIDSYNTH);
}

} // namespace rt
} // namespace drumstick
