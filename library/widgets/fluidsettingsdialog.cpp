/*
    Drumstick MIDI Sequencer C++ library
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

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardPaths>
#include <QToolButton>
#include <QToolTip>
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
const QString FluidSettingsDialog::QSTR_CHORUS_DEPTH = QStringLiteral("chorus_depth");
const QString FluidSettingsDialog::QSTR_CHORUS_LEVEL = QStringLiteral("chorus_level");
const QString FluidSettingsDialog::QSTR_CHORUS_NR = QStringLiteral("chorus_nr");
const QString FluidSettingsDialog::QSTR_CHORUS_SPEED = QStringLiteral("chorus_speed");
const QString FluidSettingsDialog::QSTR_REVERB_DAMP = QStringLiteral("reverb_damp");
const QString FluidSettingsDialog::QSTR_REVERB_LEVEL = QStringLiteral("reverb_level");
const QString FluidSettingsDialog::QSTR_REVERB_SIZE = QStringLiteral("reverb_size");
const QString FluidSettingsDialog::QSTR_REVERB_WIDTH = QStringLiteral("reverb_width");

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
    connect(ui->chorus_depth, &QAbstractSlider::valueChanged, this, [=](int val) {
        //qDebug() << "chorus depth" << val;
        setWidgetTip(ui->chorus_depth, QString::number(val / CHORUS_REVERB_VALUE_SCALE, 'f', 2));
    });
    connect(ui->chorus_level, &QAbstractSlider::valueChanged, this, [=](int val) {
        //qDebug() << "chorus level" << val;
        setWidgetTip(ui->chorus_level, QString::number(val / CHORUS_REVERB_VALUE_SCALE, 'f', 2));
    });
    connect(ui->chorus_nr, &QAbstractSlider::valueChanged, this, [=](int val) {
        setWidgetTip(ui->chorus_nr, QString::number(val));
    });
    connect(ui->chorus_speed, &QAbstractSlider::valueChanged, this, [=](int val) {
        //qDebug() << "chorus speed" << val;
        setWidgetTip(ui->chorus_speed, QString::number(val / CHORUS_REVERB_VALUE_SCALE, 'f', 2));
    });

    connect(ui->reverb_damp, &QAbstractSlider::valueChanged, this, [=](int val) {
        //qDebug() << "reverb damp" << val;
        setWidgetTip(ui->reverb_damp, QString::number(val / CHORUS_REVERB_VALUE_SCALE, 'f', 2));
    });
    connect(ui->reverb_level, &QAbstractSlider::valueChanged, this, [=](int val) {
        //qDebug() << "reverb level" << val;
        setWidgetTip(ui->reverb_level, QString::number(val / CHORUS_REVERB_VALUE_SCALE, 'f', 2));
    });
    connect(ui->reverb_size, &QAbstractSlider::valueChanged, this, [=](int val) {
        //qDebug() << "reverb size" << val;
        setWidgetTip(ui->reverb_size, QString::number(val / CHORUS_REVERB_VALUE_SCALE, 'f', 2));
    });
    connect(ui->reverb_width, &QAbstractSlider::valueChanged, this, [=](int val) {
        //qDebug() << "reverb width" << val;
        setWidgetTip(ui->reverb_width, QString::number(val / CHORUS_REVERB_VALUE_SCALE, 'f', 2));
    });

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

void FluidSettingsDialog::setWidgetTip(QWidget *w, const QString &tip)
{
    //qDebug() << Q_FUNC_INFO << tip;
    w->setToolTip(tip);
    QToolTip::showText(w->parentWidget()->mapToGlobal(w->pos()), tip);
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
    ui->gain->setText( settings->value(QSTR_GAIN, DEFAULT_GAIN).toString() );
    ui->polyphony->setText( settings->value(QSTR_POLYPHONY, DEFAULT_POLYPHONY).toString() );
    ui->soundFont->setText( settings->value(QSTR_INSTRUMENTSDEFINITION, m_defSoundFont).toString() );

    ui->chorus_depth->setValue(settings->value(QSTR_CHORUS_DEPTH, DEFAULT_CHORUS_DEPTH).toDouble()
                               * CHORUS_REVERB_VALUE_SCALE);
    ui->chorus_level->setValue(settings->value(QSTR_CHORUS_LEVEL, DEFAULT_CHORUS_LEVEL).toDouble()
                               * CHORUS_REVERB_VALUE_SCALE);
    ui->chorus_nr->setValue(settings->value(QSTR_CHORUS_NR, DEFAULT_CHORUS_NR).toInt());
    ui->chorus_speed->setValue(settings->value(QSTR_CHORUS_SPEED, DEFAULT_CHORUS_SPEED).toDouble()
                               * CHORUS_REVERB_VALUE_SCALE);

    ui->reverb_damp->setValue(settings->value(QSTR_REVERB_DAMP, DEFAULT_REVERB_DAMP).toDouble()
                              * CHORUS_REVERB_VALUE_SCALE);
    ui->reverb_level->setValue(settings->value(QSTR_REVERB_LEVEL, DEFAULT_REVERB_LEVEL).toDouble()
                               * CHORUS_REVERB_VALUE_SCALE);
    ui->reverb_size->setValue(settings->value(QSTR_REVERB_SIZE, DEFAULT_REVERB_SIZE).toDouble()
                              * CHORUS_REVERB_VALUE_SCALE);
    ui->reverb_width->setValue(settings->value(QSTR_REVERB_WIDTH, DEFAULT_REVERB_WIDTH).toDouble()
                               * CHORUS_REVERB_VALUE_SCALE);

    ui->chorus->setChecked(settings->value(QSTR_CHORUS, DEFAULT_CHORUS).toInt() != 0);
    ui->reverb->setChecked(settings->value(QSTR_REVERB, DEFAULT_REVERB).toInt() != 0);

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

    double chorus_depth(DEFAULT_CHORUS_DEPTH);
    double chorus_level(DEFAULT_CHORUS_LEVEL);
    int chorus_nr(DEFAULT_CHORUS_NR);
    double chorus_speed(DEFAULT_CHORUS_SPEED);

    double reverb_damp(DEFAULT_REVERB_DAMP);
    double reverb_level(DEFAULT_REVERB_LEVEL);
    double reverb_size(DEFAULT_REVERB_SIZE);
    double reverb_width(DEFAULT_REVERB_WIDTH);

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

    chorus_depth = ui->chorus_depth->value() / CHORUS_REVERB_VALUE_SCALE;
    chorus_level = ui->chorus_level->value() / CHORUS_REVERB_VALUE_SCALE;
    chorus_nr = ui->chorus_nr->value();
    chorus_speed = ui->chorus_speed->value() / CHORUS_REVERB_VALUE_SCALE;
    reverb_damp = ui->reverb_damp->value() / CHORUS_REVERB_VALUE_SCALE;
    reverb_level = ui->reverb_level->value() / CHORUS_REVERB_VALUE_SCALE;
    reverb_size = ui->reverb_size->value() / CHORUS_REVERB_VALUE_SCALE;
    reverb_width = ui->reverb_width->value() / CHORUS_REVERB_VALUE_SCALE;

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
    settings->setValue(QSTR_CHORUS_DEPTH, chorus_depth);
    settings->setValue(QSTR_CHORUS_LEVEL, chorus_level);
    settings->setValue(QSTR_CHORUS_NR, chorus_nr);
    settings->setValue(QSTR_CHORUS_SPEED, chorus_speed);
    settings->setValue(QSTR_REVERB_DAMP, reverb_damp);
    settings->setValue(QSTR_REVERB_LEVEL, reverb_level);
    settings->setValue(QSTR_REVERB_SIZE, reverb_size);
    settings->setValue(QSTR_REVERB_WIDTH, reverb_width);
    settings->endGroup();
    settings->sync();

    chkDriverProperties(settings.getQSettings());
}

void FluidSettingsDialog::restoreDefaults()
{
    //qDebug() << Q_FUNC_INFO;
    ui->audioDriver->setCurrentText(defaultAudioDriver());
    ui->bufferTime->setValue(DEFAULT_BUFFERTIME);
    ui->periodSize->setValue(DEFAULT_PERIODSIZE);
    ui->periods->setValue(DEFAULT_PERIODS);
    ui->sampleRate->setText(QString::number(DEFAULT_SAMPLERATE));
    ui->gain->setText(QString::number(DEFAULT_GAIN));
    ui->polyphony->setText(QString::number(DEFAULT_POLYPHONY));
    ui->soundFont->setText(m_defSoundFont);
    ui->chorus_depth->setValue(DEFAULT_CHORUS_DEPTH * CHORUS_REVERB_VALUE_SCALE);
    ui->chorus_level->setValue(DEFAULT_CHORUS_LEVEL * CHORUS_REVERB_VALUE_SCALE);
    ui->chorus_nr->setValue(DEFAULT_CHORUS_NR);
    ui->chorus_speed->setValue(DEFAULT_CHORUS_SPEED * CHORUS_REVERB_VALUE_SCALE);
    ui->reverb_damp->setValue(DEFAULT_REVERB_DAMP * CHORUS_REVERB_VALUE_SCALE);
    ui->reverb_level->setValue(DEFAULT_REVERB_LEVEL * CHORUS_REVERB_VALUE_SCALE);
    ui->reverb_size->setValue(DEFAULT_REVERB_SIZE * CHORUS_REVERB_VALUE_SCALE);
    ui->reverb_width->setValue(DEFAULT_REVERB_WIDTH * CHORUS_REVERB_VALUE_SCALE);
    ui->chorus->setChecked(DEFAULT_CHORUS != 0);
    ui->reverb->setChecked(DEFAULT_REVERB != 0);
    initBuffer();
}

void FluidSettingsDialog::showFileDialog()
{
    QDir dir(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QSTR_DATADIR, QStandardPaths::LocateDirectory));
    if (!dir.exists()) {
        dir = QDir(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QSTR_DATADIR2, QStandardPaths::LocateDirectory));
    }
    QStringList fileNames
        = QFileDialog::getOpenFileNames(this,
                                        tr("Select SoundFont"),
                                        dir.absolutePath(),
                                        tr("SoundFont Files (*.sf2 *.sf3 *.dls)"));
    if (!fileNames.isEmpty()) {
        ui->soundFont->setText(fileNames.join(';'));
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
