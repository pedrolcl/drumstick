/*
    Virtual Piano test using the MIDI Sequencer C++ library
    Copyright (C) 2006-2020, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <QNetworkInterface>
#include <QDialogButtonBox>
#include <QPushButton>

#include <drumstick/settingsfactory.h>
#include "networksettingsdialog.h"
#include "ui_networksettingsdialog.h"

/**
 * @file networksettingsdialog.cpp
 * Implementation of the Network configuration dialog
 */

namespace drumstick {
namespace widgets {

const QString QSTR_ADDRESS_IPV4 = QLatin1Literal("225.0.0.37");
const QString QSTR_ADDRESS_IPV6 = QLatin1Literal("ff12::37");

NetworkSettingsDialog::NetworkSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NetworkSettingsDialog)
{
    ui->setupUi(this);
    connect(ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::pressed,
            this, &NetworkSettingsDialog::restoreDefaults);
    connect(ui->checkIPv6, &QCheckBox::toggled, this, &NetworkSettingsDialog::toggledIPv6);
}

NetworkSettingsDialog::~NetworkSettingsDialog()
{
    delete ui;
}

void NetworkSettingsDialog::accept()
{
    writeSettings();
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

}} // namespace drumstick::widgets
