/*
    Virtual Piano test using the MIDI Sequencer C++ library
    Copyright (C) 2006-2021, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#ifndef FLUIDSETTINGSDIALOG_H
#define FLUIDSETTINGSDIALOG_H

#include <QDialog>
#include <QShowEvent>

/**
 * @file fluidsettingsdialog.h
 * Declaration of the Fluidsynth configuration dialog
 */

namespace drumstick {

namespace rt {
    class MIDIOutput;
}

namespace widgets {

namespace Ui {
    class FluidSettingsDialog;
}

class FluidSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FluidSettingsDialog(QWidget *parent = nullptr);
    ~FluidSettingsDialog();
    void readSettings();
    void writeSettings();
    void changeSoundFont(const QString& fileName);
    void getDriverProperties();

public slots:
    void accept() override;
    void showEvent(QShowEvent *event) override;
    void restoreDefaults();
    void showFileDialog();

public:
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

    static const int DEFAULT_PERIODSIZE = 3072;
    static const int DEFAULT_PERIODS = 1;
    static const double DEFAULT_SAMPLERATE;
    static const int DEFAULT_CHORUS = 0;
    static const int DEFAULT_REVERB = 0;
    static const double DEFAULT_GAIN;
    static const int DEFAULT_POLYPHONY = 32;

private:
    QString defaultAudioDriver() const;

    Ui::FluidSettingsDialog *ui;
    drumstick::rt::MIDIOutput *m_driver;
};

}} // namespace drumstick::widgets

#endif // FLUIDSETTINGSDIALOG_H
