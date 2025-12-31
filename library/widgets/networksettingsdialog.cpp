/*
    Virtual Piano test using the MIDI Sequencer C++ library
    Copyright (C) 2006-2025, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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
#include <QNetworkInterface>
#include <QPushButton>
#include <QMessageBox>

#include "networksettingsdialog.h"
#include "ui_networksettingsdialog.h"
#include <drumstick/settingsfactory.h>
#include <drumstick/backendmanager.h>

/**
 * @file networksettingsdialog.cpp
 * Implementation of the Network configuration dialog
 */

namespace drumstick { namespace widgets {

const QString NetworkSettingsDialog::QSTR_ADDRESS_IPV4 = QStringLiteral("225.0.0.37");
const QString NetworkSettingsDialog::QSTR_ADDRESS_IPV6 = QStringLiteral("ff12::37");

NetworkSettingsDialog::NetworkSettingsDialog(const bool forInput, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NetworkSettingsDialog),
    m_input(forInput)
{
    ui->setupUi(this);
    connect(ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::pressed,
            this, &NetworkSettingsDialog::restoreDefaults);
    connect(ui->checkIPv6, &QCheckBox::toggled, this, &NetworkSettingsDialog::toggledIPv6);

    drumstick::rt::BackendManager *man = drumstick::rt::lastBackendManagerInstance();
    if (m_input) {
        m_driver = man->inputBackendByName("Network");
    } else {
        m_driver = man->outputBackendByName("Network");
    }
}

NetworkSettingsDialog::~NetworkSettingsDialog()
{
    if (m_driver != nullptr) {
        if (m_input) {
            static_cast<drumstick::rt::MIDIInput*>(m_driver)->close();
        } else {
            static_cast<drumstick::rt::MIDIOutput*>(m_driver)->close();
        }
    }
    delete ui;
}

void NetworkSettingsDialog::accept()
{
    writeSettings();
    if (m_driver != nullptr) {
        QString title;
        QVariant varStatus = m_driver->property("status");
        if (varStatus.isValid()) {
            title = varStatus.toBool() ? tr("Network Initialized") : tr("Network Initialization Failed");
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

void NetworkSettingsDialog::showEvent(QShowEvent *event)
{
    readSettings();
    event->accept();
}

void NetworkSettingsDialog::readSettings()
{
    SettingsFactory settings;
    settings->beginGroup("Network");
    QString ifaceName = settings->value("interface", QString()).toString();
    bool ipv6 = settings->value("ipv6", false).toBool();
    QString address = settings->value("address", ipv6 ? QSTR_ADDRESS_IPV6 : QSTR_ADDRESS_IPV4).toString();
    settings->endGroup();

    ui->txtAddress->setText(address);
    ui->checkIPv6->setChecked(ipv6);
    ui->comboInterface->addItem(tr("Any"), "");
    foreach( const QNetworkInterface& iface,  QNetworkInterface::allInterfaces() ) {
        if ( iface.isValid() &&
             iface.flags().testFlag(QNetworkInterface::CanMulticast) &&
             iface.flags().testFlag(QNetworkInterface::IsUp) &&
             iface.flags().testFlag(QNetworkInterface::IsRunning) &&
             !iface.flags().testFlag(QNetworkInterface::IsLoopBack) ) {
            QString name = iface.name();
            QString text = iface.humanReadableName();
            ui->comboInterface->addItem(text, name);
            if (name == ifaceName) {
                ui->comboInterface->setCurrentText(text);
            }
        }
    }
    chkInitialization(settings.getQSettings());
}

void NetworkSettingsDialog::writeSettings()
{
    SettingsFactory settings;
    QString networkAddr = ui->txtAddress->text();
    QString networkIface = ui->comboInterface->currentData().toString();
    bool ipv6 = ui->checkIPv6->isChecked();

    settings->beginGroup("Network");
    settings->setValue("address", networkAddr);
    settings->setValue("interface", networkIface);
    settings->setValue("ipv6", ipv6);
    settings->endGroup();
    settings->sync();

    chkInitialization(settings.getQSettings());
}

void NetworkSettingsDialog::chkInitialization(QSettings *settings)
{
    if (m_driver != nullptr) {
        drumstick::rt::MIDIConnection conn("21928", 21928);
        if (m_input) {
            auto d = static_cast<drumstick::rt::MIDIInput*>(m_driver);
            d->close();
            d->initialize(settings);
            d->open(conn);
        } else {
            auto d = static_cast<drumstick::rt::MIDIOutput*>(m_driver);
            d->close();
            d->initialize(settings);
            d->open(conn);
        }
        QVariant varStatus = m_driver->property("status");
        if (varStatus.isValid()) {
            ui->lblStatusText->clear();
            ui->lblStatusText->setText(varStatus.toBool() ? tr("Ready") : tr("Failed") );
            ui->lblStatusIcon->setPixmap(varStatus.toBool() ? QPixmap(":/checked.png") : QPixmap(":/error.png") );
        }
    }
}

void NetworkSettingsDialog::restoreDefaults()
{
    ui->checkIPv6->setChecked(false);
    ui->txtAddress->setText(QSTR_ADDRESS_IPV4);
    ui->comboInterface->setCurrentText(tr("Any"));
}

void NetworkSettingsDialog::toggledIPv6(bool checked)
{
    ui->txtAddress->setText(checked ? QSTR_ADDRESS_IPV6 : QSTR_ADDRESS_IPV4);
}

} // namespace widgets
} // namespace drumstick
