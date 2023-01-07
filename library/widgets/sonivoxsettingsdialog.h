/*
    Virtual Piano test using the MIDI Sequencer C++ library
    Copyright (C) 2006-2023, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#ifndef SonivoxSettingsDialog_H
#define SonivoxSettingsDialog_H

#include <QDialog>
#include <QShowEvent>
#include <QSettings>

/**
 * @file sonivoxsettingsdialog.h
 * Definition of the Sonivox Synth configuration dialog
 */

namespace drumstick {

namespace rt {
    class MIDIOutput;
}

namespace widgets {

    namespace Ui {
        class SonivoxSettingsDialog;
    }

    class SonivoxSettingsDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit SonivoxSettingsDialog(QWidget *parent = nullptr);
        ~SonivoxSettingsDialog();
        void readSettings();
        void writeSettings();
        void chkDriverProperties(QSettings *settings);

        static const QString QSTR_PREFERENCES;
        static const QString QSTR_BUFFERTIME;
        static const QString QSTR_REVERBTYPE;
        static const QString QSTR_REVERBAMT;
        static const QString QSTR_CHORUSTYPE;
        static const QString QSTR_CHORUSAMT;

    public slots:
        void accept() override;
        void showEvent(QShowEvent *event) override;
        void restoreDefaults();

    private:
        Ui::SonivoxSettingsDialog *ui;
        drumstick::rt::MIDIOutput *m_driver;
    };

}} // namespace drumstick::widgets

#endif // SonivoxSettingsDialog_H
