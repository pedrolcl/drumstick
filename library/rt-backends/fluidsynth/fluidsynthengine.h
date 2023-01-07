/*
    Drumstick RT (realtime MIDI In/Out)
    Copyright (C) 2009-2023 Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#ifndef SynthEngine_H
#define SynthEngine_H

#include <QObject>
#include <QString>
#include <QList>
#include <QDir>
#include <QSettings>
#include <QMutex>
#include <drumstick/rtmidioutput.h>
#include <fluidsynth.h>

namespace drumstick { namespace rt {

class FluidSynthEngine : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString soundFont READ soundFont WRITE setSoundFont)

public:
    explicit FluidSynthEngine(QObject *parent = nullptr);
    virtual ~FluidSynthEngine();

    QString soundFont() const { return m_soundFont; }
    void setSoundFont(const QString &value);
    void appendDiagnostics(int level, const char* message);
    void stop();

    Q_INVOKABLE void initialize();
    Q_INVOKABLE void readSettings(QSettings *settings);
    Q_INVOKABLE void scanSoundFonts();
    Q_INVOKABLE void panic();
    Q_INVOKABLE void setInstrument(const int channel, int i);
    Q_INVOKABLE void noteOn(const int channel, const int midiNote, const int velocity);
    Q_INVOKABLE void noteOff(const int channel, const int midiNote, const int velocity);
    Q_INVOKABLE void controlChange(const int channel, const int ctl, const int value);
    Q_INVOKABLE void bender(const int channel, const int value);
    Q_INVOKABLE void channelPressure(const int channel, const int value);
    Q_INVOKABLE void keyPressure(const int channel, const int midiNote, const int value);
    Q_INVOKABLE void sysex(const QByteArray& data);
    Q_INVOKABLE QString version() const { return QT_STRINGIFY(VERSION); }

    MIDIConnection currentConnection() const { return m_currentConnection; }
    void close();
    void open();
    void uninitialize();
    QStringList getAudioDrivers();
    QStringList getDiagnostics();
    QString getLibVersion();
    bool getStatus();

    static const QString QSTR_FLUIDSYNTH_VERSION;

    static const QString QSTR_FLUIDSYNTH;
    static const QString QSTR_PREFERENCES;
    static const QString QSTR_INSTRUMENTSDEFINITION;
    static const QString QSTR_DATADIR;
    static const QString QSTR_DATADIR2;
    static const QString QSTR_SOUNDFONT;

    static const QString QSTR_AUDIODRIVER;
    static const QString QSTR_PERIODSIZE;
    static const QString QSTR_PERIODS;
    static const QString QSTR_SAMPLERATE;
    static const QString QSTR_CHORUS;
    static const QString QSTR_REVERB;
    static const QString QSTR_GAIN;
    static const QString QSTR_POLYPHONY;
    static const QString QSTR_DEFAULT_AUDIODRIVER;
    static const QString QSTR_BUFFERTIME;
	static const QString QSTR_PULSEAUDIO;

    static const int DEFAULT_PERIODS;
    static const int DEFAULT_PERIODSIZE;
    static const double DEFAULT_SAMPLERATE;
    static const int DEFAULT_CHORUS;
    static const int DEFAULT_REVERB;
    static const double DEFAULT_GAIN;
    static const int DEFAULT_POLYPHONY;

private:
    void scanSoundFonts(const QDir &dir);
    void retrieveAudioDrivers();
    void initializeSynth();
    void loadSoundFont();
    void retrieveDefaultSoundfont();

    int m_sfid;
    MIDIConnection m_currentConnection;
    QString m_runtimeLibraryVersion;
    QString m_soundFont;
    QString m_defSoundFont;
    fluid_settings_t* m_settings;
    fluid_synth_t* m_synth;
    fluid_audio_driver_t* m_driver;
    QStringList m_soundFontsList;
    QStringList m_audioDriversList;
    QString fs_audiodriver{ QSTR_DEFAULT_AUDIODRIVER };
    int fs_periodSize { DEFAULT_PERIODSIZE };
    int fs_periods { DEFAULT_PERIODS };
    double fs_sampleRate { DEFAULT_SAMPLERATE };
    int fs_chorus { DEFAULT_CHORUS };
    int fs_reverb { DEFAULT_REVERB };
    double fs_gain { DEFAULT_GAIN };
    int fs_polyphony { DEFAULT_POLYPHONY };
    bool m_status;
    QStringList m_diagnostics;
};

}} // namespace drumstick::rt

#endif // SynthEngine_H

