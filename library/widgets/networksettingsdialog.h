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

#ifndef NETWORKSETTINGSDIALOG_H
#define NETWORKSETTINGSDIALOG_H

#include <QDialog>
#include <QShowEvent>
#include <QSettings>

/**
 * @file networksettingsdialog.h
 * Declaration of the Network configuration dialog
 */

namespace drumstick {
namespace widgets {

    namespace Ui {
        class NetworkSettingsDialog;
    }

    class NetworkSettingsDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit NetworkSettingsDialog(const bool forInput, QWidget *parent = nullptr);
        ~NetworkSettingsDialog();
        void readSettings();
        void writeSettings();
        void chkInitialization(QSettings* settings);

        static const QString QSTR_ADDRESS_IPV4;
        static const QString QSTR_ADDRESS_IPV6;

    public slots:
        void accept() override;
        void showEvent(QShowEvent *event) override;
        void restoreDefaults();
        void toggledIPv6(bool checked);

    private:
        Ui::NetworkSettingsDialog *ui;
        QObject *m_driver;
        bool m_input;
    };

}} // namespace drumstick::widgets

#endif // NETWORKSETTINGSDIALOG_H
