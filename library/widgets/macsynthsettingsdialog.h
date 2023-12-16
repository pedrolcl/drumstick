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

#ifndef MacSynthSettingsDialog_H
#define MacSynthSettingsDialog_H

#include <QDialog>
#include <QShowEvent>
#include <QSettings>

/**
 * @file macsynthsettingsdialog.h
 * Declaration of the Mac Synth configuration dialog
 */

namespace drumstick {

namespace rt {
    class MIDIOutput;
}

namespace widgets {

namespace Ui {
    class MacSynthSettingsDialog;
}

class MacSynthSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MacSynthSettingsDialog(QWidget *parent = nullptr);
    ~MacSynthSettingsDialog();
    void readSettings();
    void writeSettings();
    void changeSoundFont(const QString& fileName);

public Q_SLOTS:
    void accept() override;
    void showEvent(QShowEvent *event) override;
    void restoreDefaults();
    void showFileDialog();

private:
    Ui::MacSynthSettingsDialog *ui;
    drumstick::rt::MIDIOutput *m_driver;
    void checkDriver(QSettings* settings);
};

}} // namespace drumstick::widgets

#endif // MacSynthSettingsDialog_H
