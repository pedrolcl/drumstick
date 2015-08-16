/*
    Drumstick RT (realtime MIDI In/Out)
    Copyright (C) 2009-2015 Pedro Lopez-Cabanillas <plcl@users.sf.net>

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
#include <fluidsynth.h>

#define cvtstr(s) #s
#define stringify(s) cvtstr(s)

const QString QSTR_FLUIDSYNTH(QLatin1String("FluidSynth"));

class SynthEngine : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString soundFont READ soundFont WRITE setSoundFont)

public:
    SynthEngine(QObject *parent = 0);
    virtual ~SynthEngine();

    QString soundFont() const { return m_soundFont; }
    void setSoundFont(const QString &value);

    Q_INVOKABLE void initialize(QSettings *settings);
    Q_INVOKABLE void readSettings(QSettings *settings);
    Q_INVOKABLE void scanSoundFonts();
    Q_INVOKABLE void panic();
    Q_INVOKABLE void setInstrument(const int channel, int i);
    Q_INVOKABLE void noteOn(const int channel, const int midiNote, const int velocity);
    Q_INVOKABLE void noteOff(const int channel, const int midiNote, const int velocity);
    Q_INVOKABLE void controlChange(const int channel, const int ctl, const int value);
    Q_INVOKABLE void bender(const int channel, const int value);
    Q_INVOKABLE QString version() const { return stringify(VERSION); }

    QString currentConnection() const { return m_currentConnection; }
    void close();
    void open();
    void uninitialize();

private:
    void scanSoundFonts(const QDir &dir);
    void initializeSynth(QSettings *settings = 0);
    void loadSoundFont();

    int m_sfid;
    QString m_currentConnection;
    QString m_soundFont;
    QString m_defSoundFont;
    fluid_settings_t* m_settings;
    fluid_synth_t* m_synth;
    fluid_audio_driver_t* m_driver;
    QStringList m_soundFontsList;
};

#endif // SynthEngine_H

