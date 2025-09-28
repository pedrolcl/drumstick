/*
    Virtual Piano test using the MIDI Sequencer C++ library
    Copyright (C) 2006-2024, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>
#include <QDir>
#include <QStandardPaths>
#include <QFileDialog>

#include "sonivoxsettingsdialog.h"
#include "ui_sonivoxsettingsdialog.h"
#include <drumstick/settingsfactory.h>
#include <drumstick/backendmanager.h>

/**
 * @file sonivoxsettingsdialog.cpp
 * Implementation of the Sonivox Synth configuration dialog
 */

namespace drumstick { namespace widgets {

const QString SonivoxSettingsDialog::QSTR_PREFERENCES = QStringLiteral("SonivoxEAS");
const QString SonivoxSettingsDialog::QSTR_BUFFERTIME = QStringLiteral("BufferTime");
const QString SonivoxSettingsDialog::QSTR_REVERBTYPE = QStringLiteral("ReverbType");
const QString SonivoxSettingsDialog::QSTR_REVERBAMT = QStringLiteral("ReverbAmt");
const QString SonivoxSettingsDialog::QSTR_CHORUSTYPE = QStringLiteral("ChorusType");
const QString SonivoxSettingsDialog::QSTR_CHORUSAMT = QStringLiteral("ChorusAmt");
const QString SonivoxSettingsDialog::QSTR_SOUNDFONT = QStringLiteral("InstrumentsDefinition");
const QString SonivoxSettingsDialog::QSTR_DATADIR = QStringLiteral("soundfonts");
const QString SonivoxSettingsDialog::QSTR_DATADIR2 = QStringLiteral("sounds/sf2");

SonivoxSettingsDialog::SonivoxSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SonivoxSettingsDialog)
{
    ui->setupUi(this);
    ui->combo_Reverb->addItem(QStringLiteral("Large Hall"), 0);
    ui->combo_Reverb->addItem(QStringLiteral("Hall"), 1);
    ui->combo_Reverb->addItem(QStringLiteral("Chamber"), 2);
    ui->combo_Reverb->addItem(QStringLiteral("Room"), 3);
    ui->combo_Reverb->addItem(QStringLiteral("None"), -1);
    ui->combo_Reverb->setCurrentIndex(4);

    ui->combo_Chorus->addItem(QStringLiteral("Preset 1"), 0);
    ui->combo_Chorus->addItem(QStringLiteral("Preset 2"), 1);
    ui->combo_Chorus->addItem(QStringLiteral("Preset 3"), 2);
    ui->combo_Chorus->addItem(QStringLiteral("Preset 4"), 3);
    ui->combo_Chorus->addItem(QStringLiteral("None"), -1);
    ui->combo_Chorus->setCurrentIndex(4);
    connect(ui->btn_soundfont, &QToolButton::clicked, this, &SonivoxSettingsDialog::showFileDialog);
    connect(ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::pressed,
            this, &SonivoxSettingsDialog::restoreDefaults);

    drumstick::rt::BackendManager *man = drumstick::rt::lastBackendManagerInstance();
    m_driver = man->outputBackendByName("SonivoxEAS");
    //qDebug() << Q_FUNC_INFO;
}

SonivoxSettingsDialog::~SonivoxSettingsDialog()
{
    //qDebug() << Q_FUNC_INFO;
    if (m_driver != nullptr) {
        m_driver->close();
    }
    delete ui;
}

void SonivoxSettingsDialog::accept()
{
    //qDebug() << Q_FUNC_INFO;
    writeSettings();
    if (m_driver != nullptr) {
        QString title;
        QVariant varStatus = m_driver->property("status");
        if (varStatus.isValid()) {
            title = varStatus.toBool() ? tr("Sonivox Initialized") : tr("Sonivox Initialization Failed");
            QVariant varDiag = m_driver->property("diagnostics");
            if (varDiag.isValid()) {
                QString text = varDiag.toStringList().join(QChar::LineFeed).trimmed();
                if (varStatus.toBool()) {
                    if (!text.isEmpty()) {
                        QMessageBox::information(this, title, text);
                    }
                } else {
                    QMessageBox::critical(this, title, text);
                    return;
                }
            }
        }
    }
    QDialog::accept();
}

void SonivoxSettingsDialog::showEvent(QShowEvent *event)
{
    //qDebug() << Q_FUNC_INFO;
    readSettings();
    event->accept();
}

void SonivoxSettingsDialog::readSettings()
{
    //qDebug() << Q_FUNC_INFO;
    SettingsFactory settings;
    settings->beginGroup(QSTR_PREFERENCES);
    int bufferTime = settings->value(QSTR_BUFFERTIME, 30).toInt();
    int reverbType = settings->value(QSTR_REVERBTYPE, 1).toInt();
    int reverbAmt = settings->value(QSTR_REVERBAMT, 25800).toInt();
    int chorusType = settings->value(QSTR_CHORUSTYPE, -1).toInt();
    int chorusAmt = settings->value(QSTR_CHORUSAMT, 0).toInt();
    QString soundfont = settings->value(QSTR_SOUNDFONT, QString()).toString();
    settings->endGroup();

    if (qEnvironmentVariableIsSet("PULSE_LATENCY_MSEC")) {
        bufferTime = qEnvironmentVariableIntValue("PULSE_LATENCY_MSEC");
    }

    ui->spnTime->setValue(bufferTime);
    ui->soundfont_dls->setText(soundfont);
    ui->dial_Reverb->setValue(reverbAmt);
    ui->dial_Chorus->setValue(chorusAmt);
    int reverbIndex = ui->combo_Reverb->findData(reverbType);
    int chorusIndex = ui->combo_Chorus->findData(chorusType);
    ui->combo_Reverb->setCurrentIndex(reverbIndex);
    ui->combo_Chorus->setCurrentIndex(chorusIndex);

    chkDriverProperties(settings.getQSettings());
}

void SonivoxSettingsDialog::writeSettings()
{
    //qDebug() << Q_FUNC_INFO;
    SettingsFactory settings;
    settings->beginGroup(QSTR_PREFERENCES);
    settings->setValue(QSTR_BUFFERTIME, ui->spnTime->value());
    settings->setValue(QSTR_REVERBTYPE, ui->combo_Reverb->currentData());
    settings->setValue(QSTR_CHORUSTYPE, ui->combo_Chorus->currentData());
    settings->setValue(QSTR_REVERBAMT, ui->dial_Reverb->value());
    settings->setValue(QSTR_CHORUSAMT, ui->dial_Chorus->value());
    settings->setValue(QSTR_SOUNDFONT, ui->soundfont_dls->text());
    settings->endGroup();
    settings->sync();
    qputenv("PULSE_LATENCY_MSEC", QByteArray::number( ui->spnTime->value() ));
    chkDriverProperties(settings.getQSettings());
}

void SonivoxSettingsDialog::chkDriverProperties(QSettings *settings)
{
    //qDebug() << Q_FUNC_INFO;
    if (m_driver != nullptr) {
        //drumstick::rt::MIDIConnection conn;
        m_driver->close();
        m_driver->initialize(settings);
        QVariant varVersion = m_driver->property("libversion");
        if (varVersion.isValid()) {
            ui->lblLibraryText->clear();
            ui->lblLibraryText->setText(varVersion.toString());
        }
        QVariant varStatus = m_driver->property("status");
        if (varStatus.isValid()) {
            ui->lblStatusText->clear();
            ui->lblStatusText->setText(varStatus.toBool() ? tr("Ready") : tr("Failed") );
            ui->lblStatusIcon->setPixmap(varStatus.toBool() ? QPixmap(":/checked.png") : QPixmap(":/error.png") );
        }
    }
}

void SonivoxSettingsDialog::restoreDefaults()
{
    ui->spnTime->setValue(30);
    ui->soundfont_dls->clear();
    ui->combo_Reverb->setCurrentIndex(1);
    ui->dial_Reverb->setValue(25800);
    ui->combo_Chorus->setCurrentIndex(4);
    ui->dial_Chorus->setValue(0);
}

void SonivoxSettingsDialog::showFileDialog()
{
    QDir dir(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QSTR_DATADIR, QStandardPaths::LocateDirectory));
    if (!dir.exists()) {
        dir = QDir(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QSTR_DATADIR2, QStandardPaths::LocateDirectory));
    }
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select SoundFont"), dir.absolutePath(), tr("SoundFont Files (*.dls)"));
    if (!fileName.isEmpty()) {
        ui->soundfont_dls->setText(fileName);
    }
}

void SonivoxSettingsDialog::changeSoundFont(const QString& fileName)
{
    readSettings();
    ui->soundfont_dls->setText(fileName);
    writeSettings();
}

} // namespace widgets
} // namespace drumstick

