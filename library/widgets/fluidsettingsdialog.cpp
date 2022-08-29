/*
    Drumstick MIDI Sequencer C++ library
    Copyright (C) 2006-2022, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QPushButton>
#include <QStandardPaths>
#include <QToolButton>
#include <QMessageBox>
#include <QVersionNumber>

#include "fluidsettingsdialog.h"
#include "ui_fluidsettingsdialog.h"
#include <drumstick/settingsfactory.h>
#include <drumstick/backendmanager.h>

/**
 * @file fluidsettingsdialog.cpp
 * Implementation of the Fluidsynth configuration dialog
 */

namespace drumstick { namespace widgets {

const QString FluidSettingsDialog::QSTR_PREFERENCES = QStringLiteral("FluidSynth");
const QString FluidSettingsDialog::QSTR_INSTRUMENTSDEFINITION = QStringLiteral("InstrumentsDefinition");
const QString FluidSettingsDialog::QSTR_DATADIR = QStringLiteral("soundfonts");
const QString FluidSettingsDialog::QSTR_DATADIR2 = QStringLiteral("sounds/sf2");
const QString FluidSettingsDialog::QSTR_AUDIODRIVER = QStringLiteral("AudioDriver");
const QString FluidSettingsDialog::QSTR_PERIODSIZE = QStringLiteral("PeriodSize");
const QString FluidSettingsDialog::QSTR_PERIODS = QStringLiteral("Periods");
const QString FluidSettingsDialog::QSTR_SAMPLERATE = QStringLiteral("SampleRate");
const QString FluidSettingsDialog::QSTR_CHORUS = QStringLiteral("Chorus");
const QString FluidSettingsDialog::QSTR_REVERB = QStringLiteral("Reverb");
const QString FluidSettingsDialog::QSTR_GAIN = QStringLiteral("Gain");
const QString FluidSettingsDialog::QSTR_POLYPHONY = QStringLiteral("Polyphony");
const QString FluidSettingsDialog::QSTR_BUFFERTIME = QStringLiteral("BufferTime");
const QString FluidSettingsDialog::QSTR_PULSEAUDIO = QStringLiteral("pulseaudio");

FluidSettingsDialog::FluidSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FluidSettingsDialog)
{
    ui->setupUi(this);
    connect(ui->audioDriver, &QComboBox::currentTextChanged, this, &FluidSettingsDialog::audioDriverChanged);
    connect(ui->bufferTime, QOverload<int>::of(&QSpinBox::valueChanged), this, &FluidSettingsDialog::bufferTimeChanged);
    connect(ui->periodSize, QOverload<int>::of(&QSpinBox::valueChanged), this, &FluidSettingsDialog::bufferSizeChanged);
    connect(ui->periods, QOverload<int>::of(&QSpinBox::valueChanged), this, &FluidSettingsDialog::bufferSizeChanged);
    connect(ui->btnFile, &QToolButton::clicked, this, &FluidSettingsDialog::showFileDialog);
    connect(ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked,
            this, &FluidSettingsDialog::restoreDefaults);
    auto sampleRateValidator = new QDoubleValidator(8000.0, 96000.0, 1, this);
    sampleRateValidator->setNotation(QDoubleValidator::StandardNotation);
    sampleRateValidator->setLocale(QLocale::c());
    ui->sampleRate->setValidator(sampleRateValidator);
    auto gainValidator = new QDoubleValidator(0.1, 10.0, 2, this);
    gainValidator->setNotation(QDoubleValidator::StandardNotation);
    gainValidator->setLocale(QLocale::c());
    ui->gain->setValidator(gainValidator);
    auto polyphonyValidator = new QIntValidator(1, 65535, this);
    ui->polyphony->setValidator(polyphonyValidator);

    drumstick::rt::BackendManager man;
    m_driver = man.outputBackendByName("FluidSynth");
    if (m_driver != nullptr) {
        QVariant v = m_driver->property("audiodrivers");
        if (v.isValid()) {
            ui->audioDriver->blockSignals(true);
            ui->audioDriver->clear();
            ui->audioDriver->addItems(v.toStringList());
            ui->audioDriver->blockSignals(false);
        }
        v = m_driver->property("soundfont");
        if (v.isValid()) {
            m_defSoundFont = v.toString();
        }
    }
    ui->bufferTime->blockSignals(true);
    ui->periodSize->blockSignals(true);
    ui->periods->blockSignals(true);
    //qDebug() << Q_FUNC_INFO;
}

FluidSettingsDialog::~FluidSettingsDialog()
{
    //qDebug() << Q_FUNC_INFO;
    if (m_driver != nullptr) {
        m_driver->close();
    }
    delete ui;
}

bool FluidSettingsDialog::checkRanges() const
{
    //qDebug() << Q_FUNC_INFO;
    if (ui->gain->hasAcceptableInput()) {
        ui->gain->deselect();
    } else {
        ui->gain->selectAll();
    }
    if (ui->polyphony->hasAcceptableInput()) {
        ui->polyphony->deselect();
    } else {
        ui->polyphony->selectAll();
    }
    if (ui->sampleRate->hasAcceptableInput()) {
        ui->sampleRate->deselect();
    } else {
        ui->sampleRate->selectAll();
    }
    return
        ui->bufferTime->hasAcceptableInput() &&
        ui->periodSize->hasAcceptableInput() &&
        ui->periods->hasAcceptableInput() &&
        ui->gain->hasAcceptableInput() &&
        ui->polyphony->hasAcceptableInput() &&
        ui->sampleRate->hasAcceptableInput();
}

void FluidSettingsDialog::accept()
{
    //qDebug() << Q_FUNC_INFO;
    if (checkRanges()) {
        writeSettings();
        if (m_driver != nullptr) {
            QString title;
            QVariant varStatus = m_driver->property("status");
            if (varStatus.isValid()) {
                title = varStatus.toBool() ? tr("FluidSynth Initialized") : tr("FluidSynth Initialization Failed");
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
}

void FluidSettingsDialog::showEvent(QShowEvent *event)
{
    readSettings();
    event->accept();
}

QString FluidSettingsDialog::defaultAudioDriver() const
{
    const QString QSTR_DEFAULT_AUDIODRIVER =
#if defined(Q_OS_WIN)
        QLatin1String("wasapi");
#elif defined(Q_OS_OSX)
        QLatin1String("coreaudio");
#elif defined(Q_OS_LINUX)
        QSTR_PULSEAUDIO;
#else
        QLatin1String("oss");
#endif
    return QSTR_DEFAULT_AUDIODRIVER;
}

void FluidSettingsDialog::chkDriverProperties(QSettings *settings)
{
    //qDebug() << Q_FUNC_INFO;
    if (m_driver != nullptr) {
        drumstick::rt::MIDIConnection conn;
        m_driver->close();
        m_driver->initialize(settings);
        m_driver->open(conn);

        QVariant drivers = m_driver->property("audiodrivers");
        if (drivers.isValid()) {
            auto text = ui->audioDriver->currentText();
            ui->audioDriver->blockSignals(true);
            ui->audioDriver->clear();
            ui->audioDriver->addItems(drivers.toStringList());
            ui->audioDriver->setCurrentText(text);
            ui->audioDriver->blockSignals(false);
        }
        ui->lblVersion->clear();
        ui->lblVersion->setText(driverVersion());
        QVariant varStatus = m_driver->property("status");
        if (varStatus.isValid()) {
            ui->lblStatus->clear();
            ui->lblStatus->setText(varStatus.toBool() ? tr("Ready") : tr("Failed") );
            ui->lblStatusIcon->setPixmap(varStatus.toBool() ? QPixmap(":/checked.png") : QPixmap(":/error.png") );
        }
    }
}

void drumstick::widgets::FluidSettingsDialog::initBuffer()
{
    if ((ui->audioDriver->currentText() == QSTR_PULSEAUDIO) && driverVersionLessThan_2_2_8()) {
        //qDebug() << Q_FUNC_INFO << QSTR_PULSEAUDIO << driverVersion();
        int bufferTime = ui->bufferTime->value();
        int minBufTime = ui->bufferTime->minimum();
        if (bufferTime < minBufTime) {
            bufferTime = minBufTime;
        }
        ui->bufferTime->setValue( bufferTime );
        bufferTimeChanged( bufferTime );
    } else {
        //qDebug() << Q_FUNC_INFO;
        bufferSizeChanged();
    }
}

QString FluidSettingsDialog::driverVersion() const
{
    static QString result;
    if (m_driver != nullptr && result.isEmpty()) {
        QVariant varVersion = m_driver->property("libversion");
        if (varVersion.isValid()) {
            result = varVersion.toString();
        }
    }
    return result;
}

bool FluidSettingsDialog::driverVersionLessThan_2_2_8()
{
    static const QVersionNumber check_2_2_8(2, 2, 8);
    QVersionNumber driverV = QVersionNumber::fromString(driverVersion());
    return driverV < check_2_2_8;
}

void FluidSettingsDialog::readSettings()
{
    //qDebug() << Q_FUNC_INFO;
    SettingsFactory settings;
    settings->beginGroup(QSTR_PREFERENCES);
    ui->audioDriver->setCurrentText( settings->value(QSTR_AUDIODRIVER, defaultAudioDriver()).toString() );
    ui->bufferTime->setValue( settings->value(QSTR_BUFFERTIME, DEFAULT_BUFFERTIME).toInt() );
    ui->periodSize->setValue( settings->value(QSTR_PERIODSIZE, DEFAULT_PERIODSIZE).toInt() );
    ui->periods->setValue( settings->value(QSTR_PERIODS, DEFAULT_PERIODS).toInt() );
    ui->sampleRate->setText( settings->value(QSTR_SAMPLERATE, DEFAULT_SAMPLERATE).toString() );
    ui->chorus->setChecked( settings->value(QSTR_CHORUS, DEFAULT_CHORUS).toInt() != 0 );
    ui->reverb->setChecked( settings->value(QSTR_REVERB, DEFAULT_REVERB).toInt() != 0 );
    ui->gain->setText( settings->value(QSTR_GAIN, DEFAULT_GAIN).toString() );
    ui->polyphony->setText( settings->value(QSTR_POLYPHONY, DEFAULT_POLYPHONY).toString() );
    ui->soundFont->setText( settings->value(QSTR_INSTRUMENTSDEFINITION, m_defSoundFont).toString() );
    settings->endGroup();

    audioDriverChanged( ui->audioDriver->currentText() );
    chkDriverProperties(settings.getQSettings());
}

void FluidSettingsDialog::writeSettings()
{
    //qDebug() << Q_FUNC_INFO;
    SettingsFactory settings;
    QString audioDriver;
    QString soundFont(m_defSoundFont);
    int     bufferTime(DEFAULT_BUFFERTIME);
    int     periodSize(DEFAULT_PERIODSIZE);
    int     periods(DEFAULT_PERIODS);
    double  sampleRate(DEFAULT_SAMPLERATE);
    int     chorus(DEFAULT_CHORUS);
    int     reverb(DEFAULT_REVERB);
    double  gain(DEFAULT_GAIN);
    int     polyphony(DEFAULT_POLYPHONY);

    audioDriver = ui->audioDriver->currentText();
    if (audioDriver.isEmpty()) {
        audioDriver = defaultAudioDriver();
    }
    soundFont = ui->soundFont->text();
    bufferTime = ui->bufferTime->value();
    periodSize = ui->periodSize->value();
    periods = ui->periods->value();
    sampleRate = ui->sampleRate->text().toDouble();
    chorus = (ui->chorus->isChecked() ? 1 : 0);
    reverb = (ui->reverb->isChecked() ? 1 : 0);
    gain = ui->gain->text().toDouble();
    polyphony = ui->polyphony->text().toInt();

    settings->beginGroup(QSTR_PREFERENCES);
    settings->setValue(QSTR_INSTRUMENTSDEFINITION, soundFont);
    settings->setValue(QSTR_AUDIODRIVER, audioDriver);
    settings->setValue(QSTR_BUFFERTIME, bufferTime);
    settings->setValue(QSTR_PERIODSIZE, periodSize);
    settings->setValue(QSTR_PERIODS, periods);
    settings->setValue(QSTR_SAMPLERATE, sampleRate);
    settings->setValue(QSTR_CHORUS, chorus);
    settings->setValue(QSTR_REVERB, reverb);
    settings->setValue(QSTR_GAIN, gain);
    settings->setValue(QSTR_POLYPHONY, polyphony);
    settings->endGroup();
    settings->sync();

    chkDriverProperties(settings.getQSettings());
}

void FluidSettingsDialog::restoreDefaults()
{
    //qDebug() << Q_FUNC_INFO;
    ui->audioDriver->setCurrentText( defaultAudioDriver() );
    ui->bufferTime->setValue( DEFAULT_BUFFERTIME );
    ui->periodSize->setValue( DEFAULT_PERIODSIZE );
    ui->periods->setValue( DEFAULT_PERIODS );
    ui->sampleRate->setText( QString::number( DEFAULT_SAMPLERATE ));
    ui->chorus->setChecked( DEFAULT_CHORUS != 0 );
    ui->reverb->setChecked( DEFAULT_REVERB != 0 );
    ui->gain->setText( QString::number( DEFAULT_GAIN ) );
    ui->polyphony->setText( QString::number( DEFAULT_POLYPHONY ));
    ui->soundFont->setText( m_defSoundFont );
    initBuffer();
}

void FluidSettingsDialog::showFileDialog()
{
    QDir dir(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QSTR_DATADIR, QStandardPaths::LocateDirectory));
    if (!dir.exists()) {
        dir = QDir(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QSTR_DATADIR2, QStandardPaths::LocateDirectory));
    }
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select SoundFont"), dir.absolutePath(), tr("SoundFont Files (*.sf2 *.sf3 *.dls)"));
    if (!fileName.isEmpty()) {
        ui->soundFont->setText(fileName);
    }
}

void FluidSettingsDialog::audioDriverChanged(const QString &text)
{
    //qDebug() << Q_FUNC_INFO << text;
    if ((text == QSTR_PULSEAUDIO) && driverVersionLessThan_2_2_8()) {
        ui->bufferTime->setDisabled(false);
        ui->bufferTime->blockSignals(false);
        ui->periodSize->setDisabled(true);
        ui->periodSize->blockSignals(true);
        ui->periods->setVisible(false);
        ui->periods->setDisabled(true);
        ui->periods->blockSignals(true);
    } else {
        ui->bufferTime->setDisabled(true);
        ui->bufferTime->blockSignals(true);
        ui->periodSize->setDisabled(false);
        ui->periodSize->blockSignals(false);
        ui->periods->setVisible(true);
        ui->periods->setDisabled(false);
        ui->periods->blockSignals(false);
    }
    initBuffer();
}

void FluidSettingsDialog::bufferTimeChanged(int value)
{
    double rate = ui->sampleRate->text().toDouble();
    int size = qRound( value * rate / 1000.0 );
    ui->periodSize->setValue( size );
    ui->periods->setValue( ui->periods->minimum() );
    //qDebug() << Q_FUNC_INFO << "time:" << value << "rate:" << rate << "size:" << size;
}

void FluidSettingsDialog::bufferSizeChanged()
{
    QString audioDriver = ui->audioDriver->currentText();
    double rate = ui->sampleRate->text().toDouble();
    int size = ui->periodSize->value();
    if ((audioDriver != QSTR_PULSEAUDIO) || !driverVersionLessThan_2_2_8()) {
        size *= ui->periods->value();
    }
    int ms = qRound( 1000.0 * size / rate );
    ui->bufferTime->setValue(ms);
    //qDebug() << Q_FUNC_INFO << "time:" << ms << "rate:" << rate << "size:" << size;
}

void FluidSettingsDialog::changeSoundFont(const QString& fileName)
{
    readSettings();
    ui->soundFont->setText(fileName);
    writeSettings();
}

} // namespace widgets
} // namespace drumstick
