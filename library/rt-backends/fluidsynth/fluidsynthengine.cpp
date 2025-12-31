/*
    Drumstick RT (realtime MIDI In/Out)
    Copyright (C) 2009-2025 Pedro Lopez-Cabanillas <plcl@users.sf.net>

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
#include <QVersionNumber>
#include <drumstick/rtmidioutput.h>
#include "fluidsynthengine.h"

namespace drumstick { namespace rt {

const QString FluidSynthEngine::QSTR_FLUIDSYNTH_VERSION = QStringLiteral(FLUIDSYNTH_VERSION);
const QString FluidSynthEngine::QSTR_FLUIDSYNTH = QStringLiteral("FluidSynth");
const QString FluidSynthEngine::QSTR_PREFERENCES = QStringLiteral("FluidSynth");
const QString FluidSynthEngine::QSTR_INSTRUMENTSDEFINITION = QStringLiteral("InstrumentsDefinition");
const QString FluidSynthEngine::QSTR_DATADIR = QStringLiteral("soundfonts");
const QString FluidSynthEngine::QSTR_DATADIR2 = QStringLiteral("sounds/sf2");
const QString FluidSynthEngine::QSTR_SOUNDFONT = QStringLiteral("default.sf2");
const QString FluidSynthEngine::QSTR_PULSEAUDIO = QStringLiteral("pulseaudio");
const QString FluidSynthEngine::QSTR_JACK = QStringLiteral("jack");

const QString FluidSynthEngine::QSTR_AUDIODRIVER = QStringLiteral("AudioDriver");
const QString FluidSynthEngine::QSTR_BUFFERTIME = QStringLiteral("BufferTime");
const QString FluidSynthEngine::QSTR_PERIODSIZE = QStringLiteral("PeriodSize");
const QString FluidSynthEngine::QSTR_PERIODS = QStringLiteral("Periods");
const QString FluidSynthEngine::QSTR_SAMPLERATE = QStringLiteral("SampleRate");
const QString FluidSynthEngine::QSTR_CHORUS = QStringLiteral("Chorus");
const QString FluidSynthEngine::QSTR_REVERB = QStringLiteral("Reverb");
const QString FluidSynthEngine::QSTR_GAIN = QStringLiteral("Gain");
const QString FluidSynthEngine::QSTR_POLYPHONY = QStringLiteral("Polyphony");

const QString FluidSynthEngine::QSTR_CHORUS_DEPTH = QStringLiteral("chorus_depth");
const QString FluidSynthEngine::QSTR_CHORUS_LEVEL = QStringLiteral("chorus_level");
const QString FluidSynthEngine::QSTR_CHORUS_NR = QStringLiteral("chorus_nr");
const QString FluidSynthEngine::QSTR_CHORUS_SPEED = QStringLiteral("chorus_speed");
const QString FluidSynthEngine::QSTR_REVERB_DAMP = QStringLiteral("reverb_damp");
const QString FluidSynthEngine::QSTR_REVERB_LEVEL = QStringLiteral("reverb_level");
const QString FluidSynthEngine::QSTR_REVERB_SIZE = QStringLiteral("reverb_size");
const QString FluidSynthEngine::QSTR_REVERB_WIDTH = QStringLiteral("reverb_width");

const QString FluidSynthEngine::QSTR_DEFAULT_AUDIODRIVER =
#if defined(Q_OS_LINUX)
    QSTR_PULSEAUDIO;
#elif defined(Q_OS_WIN)
    QStringLiteral("wasapi");
#elif defined(Q_OS_OSX)
    QStringLiteral("coreaudio");
#else
    QStringLiteral("oss");
#endif
const int FluidSynthEngine::DEFAULT_PERIODS = 8;
const int FluidSynthEngine::DEFAULT_PERIODSIZE = 512;
const double FluidSynthEngine::DEFAULT_SAMPLERATE = 44100.0;
const int FluidSynthEngine::DEFAULT_CHORUS = 0;
const int FluidSynthEngine::DEFAULT_REVERB = 1;
const double FluidSynthEngine::DEFAULT_GAIN = 1.0;
const int FluidSynthEngine::DEFAULT_POLYPHONY = 256;

static void
SynthEngine_log_function(int level, const char* message, void* data)
{
    FluidSynthEngine *classInstance = static_cast<FluidSynthEngine*>(data);
    classInstance->appendDiagnostics(level, message);
}

FluidSynthEngine::FluidSynthEngine(QObject *parent)
    : QObject(parent),
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

FluidSynthEngine::~FluidSynthEngine()
{
    //qDebug() << Q_FUNC_INFO;
    uninitialize();
}

void FluidSynthEngine::uninitialize()
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

void FluidSynthEngine::initializeSynth()
{
    uninitialize();
    //qDebug() << Q_FUNC_INFO << fs_audiodriver << fs_periodSize << fs_periods << qEnvironmentVariableIntValue("PULSE_LATENCY_MSEC");
    m_settings = ::new_fluid_settings();
    ::fluid_settings_setstr(m_settings, "audio.driver", qPrintable(fs_audiodriver));
    ::fluid_settings_setint(m_settings, "audio.period-size", fs_periodSize);
    ::fluid_settings_setint(m_settings, "audio.periods", fs_periods);
	if (fs_audiodriver == QSTR_PULSEAUDIO) {
		::fluid_settings_setint(m_settings, "audio.pulseaudio.adjust-latency", 0);
    } else if (fs_audiodriver == QSTR_JACK) {
        ::fluid_settings_setint(m_settings, "audio.jack.autoconnect", 1);
    }

    ::fluid_settings_setnum(m_settings, "synth.reverb.damp", fs_reverb_damp);
    ::fluid_settings_setnum(m_settings, "synth.reverb.level", fs_reverb_level);
    ::fluid_settings_setnum(m_settings, "synth.reverb.room-size", fs_reverb_size);
    ::fluid_settings_setnum(m_settings, "synth.reverb.width", fs_reverb_width);

    ::fluid_settings_setnum(m_settings, "synth.chorus.depth", fs_chorus_depth);
    ::fluid_settings_setnum(m_settings, "synth.chorus.level", fs_chorus_level);
    ::fluid_settings_setint(m_settings, "synth.chorus.nr", fs_chorus_nr);
    ::fluid_settings_setnum(m_settings, "synth.chorus.speed", fs_chorus_speed);

    ::fluid_settings_setnum(m_settings, "synth.sample-rate", fs_sampleRate);
    ::fluid_settings_setint(m_settings, "synth.chorus.active", fs_chorus);
    ::fluid_settings_setint(m_settings, "synth.reverb.active", fs_reverb);
    ::fluid_settings_setnum(m_settings, "synth.gain", fs_gain);
    ::fluid_settings_setint(m_settings, "synth.polyphony", fs_polyphony);
    m_synth = ::new_fluid_synth(m_settings);
    m_driver = ::new_fluid_audio_driver(m_settings, m_synth);
}

void FluidSynthEngine::setInstrument(int channel, int pgm)
{
    ::fluid_synth_program_change(m_synth, channel, pgm);
}

void FluidSynthEngine::noteOn(int channel, int midiNote, int velocity)
{
    ::fluid_synth_noteon(m_synth, channel, midiNote, velocity);
}

void FluidSynthEngine::noteOff(int channel, int midiNote, int /*velocity*/)
{
    ::fluid_synth_noteoff(m_synth, channel, midiNote);
}

void FluidSynthEngine::loadSoundFont()
{
    if (!m_sfids.isEmpty()) {
        foreach (const int id, m_sfids) {
            if (id > -1) {
                ::fluid_synth_sfunload(m_synth, unsigned(id), 1);
            }
        }
        m_sfids.clear();
    }
    const QStringList soundfonts = m_soundFont.split(';', Qt::SkipEmptyParts);
    foreach (const QString &sf, soundfonts) {
        int id = ::fluid_synth_sfload(m_synth, qPrintable(sf), 1);
        if (id > -1) {
            m_sfids.append(id);
        }
    }
}

void FluidSynthEngine::retrieveDefaultSoundfont()
{
    /* find the default configured soundfont */
    if (m_defSoundFont.isEmpty()) {
        char *psValue = nullptr;
        int ok = ::fluid_settings_dupstr(m_settings, "synth.default-soundfont", &psValue);
        if (ok == FLUID_OK) {
            m_defSoundFont = QString(psValue);
            ::fluid_free(psValue);
        }
    }
}

void FluidSynthEngine::initialize()
{
    //qDebug() << Q_FUNC_INFO;
    initializeSynth();
    retrieveAudioDrivers();
    retrieveDefaultSoundfont();
    if (m_defSoundFont.isEmpty()) {
        scanSoundFonts();
    }
    if (m_soundFont.isEmpty() && !m_defSoundFont.isEmpty()) {
        m_soundFont = m_defSoundFont;
    }
    loadSoundFont();
    m_status = (m_synth != nullptr) && (m_driver != nullptr) && (!m_sfids.isEmpty());
}

void FluidSynthEngine::panic()
{
    ::fluid_synth_system_reset(m_synth);
}

void FluidSynthEngine::controlChange(const int channel, const int midiCtl, const int value)
{
    ::fluid_synth_cc(m_synth, channel, midiCtl, value);
}

void FluidSynthEngine::bender(const int channel, const int value)
{
    ::fluid_synth_pitch_bend(m_synth, channel, value + 8192);
}

void FluidSynthEngine::channelPressure(const int channel, const int value)
{
    ::fluid_synth_channel_pressure(m_synth, channel, value);
}

void FluidSynthEngine::keyPressure(const int channel, const int midiNote, const int value)
{
    static const QVersionNumber versionCheck(2,0,0);
    QVersionNumber fluidVersion = QVersionNumber::fromString(getLibVersion());
    if (fluidVersion >= versionCheck) {
        ::fluid_synth_key_pressure(m_synth, channel, midiNote, value);
    }
}

void FluidSynthEngine::sysex(const QByteArray &data)
{
    const unsigned char SYSEX = 0xf0;
    const unsigned char EOX = 0xf7;
    QByteArray d(data);
    if (d.startsWith(SYSEX)) {
        d.remove(0, 1);
    }
    if (d.endsWith(EOX)) {
        d.chop(1);
    }
    ::fluid_synth_sysex(m_synth, d.data(), d.length(), nullptr, nullptr, nullptr, 0);
}

void FluidSynthEngine::setSoundFont(const QString &value)
{
    if (value != m_soundFont) {
        m_soundFont = value;
        loadSoundFont();
    }
}

void FluidSynthEngine::appendDiagnostics(int level, const char *message)
{
    static const QMap<int,QString> prefix {
        {fluid_log_level::FLUID_DBG,  tr("Debug")},
        {fluid_log_level::FLUID_ERR,  tr("Error")},
        {fluid_log_level::FLUID_WARN, tr("Warning")},
        {fluid_log_level::FLUID_INFO, tr("Information")}
    };
    m_diagnostics.append(prefix[level]+": "+message);
}

void FluidSynthEngine::stop()
{
    //qDebug() << Q_FUNC_INFO;
    uninitialize();
}

QStringList FluidSynthEngine::getAudioDrivers()
{
    return m_audioDriversList;
}

QStringList FluidSynthEngine::getDiagnostics()
{
    return m_diagnostics;
}

QString FluidSynthEngine::getLibVersion()
{
    return m_runtimeLibraryVersion;
}

bool FluidSynthEngine::getStatus()
{
    return m_status;
}

void FluidSynthEngine::writeSettings(QSettings *settings)
{
    if (settings != nullptr) {
        settings->beginGroup(QSTR_PREFERENCES);
        settings->setValue(QSTR_INSTRUMENTSDEFINITION, m_soundFont);
        settings->setValue(QSTR_AUDIODRIVER, fs_audiodriver);
        settings->setValue(QSTR_PERIODSIZE, fs_periodSize);
        settings->setValue(QSTR_PERIODS, fs_periods);
        settings->setValue(QSTR_SAMPLERATE, fs_sampleRate);
        settings->setValue(QSTR_CHORUS, fs_chorus);
        settings->setValue(QSTR_REVERB, fs_reverb);
        settings->setValue(QSTR_GAIN, fs_gain);
        settings->setValue(QSTR_POLYPHONY, fs_polyphony);
        int bufferTime = 1000 * fs_periodSize * fs_periods / fs_sampleRate;
        settings->setValue(QSTR_BUFFERTIME, bufferTime);

        settings->setValue(QSTR_REVERB_DAMP, fs_reverb_damp);
        settings->setValue(QSTR_REVERB_LEVEL, fs_reverb_level);
        settings->setValue(QSTR_REVERB_SIZE, fs_reverb_size);
        settings->setValue(QSTR_REVERB_WIDTH, fs_reverb_width);

        settings->setValue(QSTR_CHORUS_DEPTH, fs_chorus_depth);
        settings->setValue(QSTR_CHORUS_LEVEL, fs_chorus_level);
        settings->setValue(QSTR_CHORUS_NR, fs_chorus_nr);
        settings->setValue(QSTR_CHORUS_SPEED, fs_chorus_speed);

        settings->endGroup();
    }
}

void FluidSynthEngine::scanSoundFonts(const QDir &initialDir)
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

void FluidSynthEngine::retrieveAudioDrivers()
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

void FluidSynthEngine::scanSoundFonts()
{
    if (m_defSoundFont.isEmpty()) {
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
        if (m_soundFontsList.length() > 0) {
            m_defSoundFont = m_soundFontsList.first();
        }
    }
}

void FluidSynthEngine::readSettings(QSettings *settings)
{
    m_sfids.clear();
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

    fs_reverb_damp = settings->value(QSTR_REVERB_DAMP, DEFAULT_REVERB_DAMP).toDouble();
    fs_reverb_level = settings->value(QSTR_REVERB_LEVEL, DEFAULT_REVERB_LEVEL).toDouble();
    fs_reverb_size = settings->value(QSTR_REVERB_SIZE, DEFAULT_REVERB_SIZE).toDouble();
    fs_reverb_width = settings->value(QSTR_REVERB_WIDTH, DEFAULT_REVERB_WIDTH).toDouble();

    fs_chorus_depth = settings->value(QSTR_CHORUS_DEPTH, DEFAULT_CHORUS_DEPTH).toDouble();
    fs_chorus_level = settings->value(QSTR_CHORUS_LEVEL, DEFAULT_CHORUS_LEVEL).toDouble();
    fs_chorus_nr = settings->value(QSTR_CHORUS_NR, DEFAULT_CHORUS_NR).toInt();
    fs_chorus_speed = settings->value(QSTR_CHORUS_SPEED, DEFAULT_CHORUS_SPEED).toDouble();

    settings->endGroup();
    //qDebug() << Q_FUNC_INFO << "audioDriver:" << fs_audiodriver << "buffer" << fs_periodSize << '*' << fs_periods;
	if (fs_audiodriver == QSTR_PULSEAUDIO) {
		int bufferTime = 1000 * fs_periodSize * fs_periods / fs_sampleRate;
		qputenv("PULSE_LATENCY_MSEC", QByteArray::number( bufferTime ) );
		//qDebug() << Q_FUNC_INFO << "$PULSE_LATENCY_MSEC=" << bufferTime;
	}
}

void FluidSynthEngine::close()
{
    //qDebug() << Q_FUNC_INFO;
    m_currentConnection = MIDIConnection();
}

void FluidSynthEngine::
open()
{
    //qDebug() << Q_FUNC_INFO;
    m_currentConnection = MIDIConnection(QSTR_FLUIDSYNTH, QSTR_FLUIDSYNTH);
}

} // namespace rt
} // namespace drumstick
